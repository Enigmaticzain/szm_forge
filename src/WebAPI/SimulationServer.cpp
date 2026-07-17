#include "SimulationServer.hpp"
#include "UuidUtils.hpp"
#include <chrono>
#include <thread>
#include <algorithm>

namespace SZM::WebAPI {

SimulationServer::SimulationServer()
    : SimulationServer(ServerConfig{}) {}

SimulationServer::SimulationServer(const ServerConfig& config)
    : m_Config(config), m_IsRunning(false) {}

SimulationServer::~SimulationServer() {
    Stop();
}

bool SimulationServer::Start() {
    if (m_IsRunning) return false;
    
    m_IsRunning = true;

    // Spawn worker threads
    for (uint32_t i = 0; i < m_Config.maxConcurrentJobs; ++i) {
        m_WorkerThreads.emplace_back(&SimulationServer::WorkerThreadMain, this);
    }

    // TODO: Start REST API server (using FastAPI bridge or cpp-httplib)
    // Listen on m_Config.host:m_Config.port
    // Setup endpoints:
    //   POST /jobs - Submit job
    //   GET /jobs/{id} - Get job status
    //   DELETE /jobs/{id} - Cancel job
    //   GET /materials - List materials
    //   etc.

    return true;
}

void SimulationServer::Stop() {
    if (!m_IsRunning) return;
    
    m_IsRunning = false;
    m_JobAvailable.notify_all();

    // Wait for worker threads
    for (auto& thread : m_WorkerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_WorkerThreads.clear();
}

std::string SimulationServer::SubmitJob(const SimulationJob& job) {
    if (!ValidateJobParameters(job)) {
        return "";
    }

    std::string jobId = GenerateJobId();
    
    auto newJob = job;
    newJob.jobId = jobId;
    newJob.createdTime = std::chrono::system_clock::now().time_since_epoch().count();
    newJob.status = JobStatus::Pending;

    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        if (m_JobQueue.size() >= m_Config.maxQueuedJobs) {
            return "";  // Queue full
        }
        m_JobQueue.push(newJob);
    }
    {
        std::lock_guard<std::mutex> lock(m_HistoryMutex);
        m_JobHistory[jobId] = newJob;
    }

    m_JobAvailable.notify_one();
    return jobId;
}

bool SimulationServer::CancelJob(const std::string& jobId) {
    std::lock_guard<std::mutex> lock(m_HistoryMutex);
    auto it = m_JobHistory.find(jobId);
    if (it != m_JobHistory.end()) {
        if (it->second.status == JobStatus::Pending || it->second.status == JobStatus::Running) {
            it->second.status = JobStatus::Cancelled;
            return true;
        }
    }
    return false;
}

SimulationServer::SimulationJob SimulationServer::GetJobStatus(const std::string& jobId) {
    std::lock_guard<std::mutex> lock(m_HistoryMutex);
    auto it = m_JobHistory.find(jobId);
    if (it != m_JobHistory.end()) {
        return it->second;
    }
    return {};
}

std::vector<SimulationServer::SimulationJob> SimulationServer::GetComponentJobs(uint32_t componentId) {
    std::vector<SimulationJob> result;
    std::lock_guard<std::mutex> lock(m_HistoryMutex);
    
    for (const auto& [id, job] : m_JobHistory) {
        if (job.componentId == componentId) {
            result.push_back(job);
        }
    }
    return result;
}

std::vector<SimulationServer::SimulationJob> SimulationServer::GetJobHistory(size_t limit, size_t offset) {
    std::vector<SimulationJob> result;
    std::lock_guard<std::mutex> lock(m_HistoryMutex);
    
    size_t count = 0;
    for (const auto& [id, job] : m_JobHistory) {
        if (count >= offset && result.size() < limit) {
            result.push_back(job);
        }
        count++;
    }
    return result;
}

json SimulationServer::GetMaterials(const std::string& filter) {
    // TODO: Query material database
    return json::array();
}

json SimulationServer::GetMaterialProperties(const std::string& materialName) {
    // TODO: Query material database
    return json::object();
}

bool SimulationServer::UpdateMaterialProperties(const std::string& materialName, const json& properties) {
    // TODO: Update material database
    return true;
}

json SimulationServer::GetComponentInfo(uint32_t componentId) {
    // TODO: Query scene graph and return component info
    return json::object();
}

json SimulationServer::CheckInterference(uint32_t componentId1, uint32_t componentId2) {
    // TODO: Run interference check
    return json::object();
}

json SimulationServer::CalculateMassProperties(uint32_t componentId) {
    // TODO: Calculate mass, COM, inertia
    return json::object();
}

json SimulationServer::GetJobResult(const std::string& jobId) {
    std::lock_guard<std::mutex> lock(m_CacheMutex);
    auto it = m_ResultCache.find(jobId);
    if (it != m_ResultCache.end()) {
        return it->second;
    }
    return json::object();
}

bool SimulationServer::StreamJobResult(const std::string& jobId, 
                                      std::function<void(const json&)> callback) {
    // TODO: Stream results as available (WebSocket compatible)
    return true;
}

void SimulationServer::ClearResultCache() {
    std::lock_guard<std::mutex> lock(m_CacheMutex);
    m_ResultCache.clear();
}

json SimulationServer::GetServerStats() {
    return {
        {"isRunning", m_IsRunning},
        {"queueDepth", GetQueueDepth()},
        {"jobHistorySize", m_JobHistory.size()},
        {"cacheSize", m_ResultCache.size()},
        {"maxConcurrent", m_Config.maxConcurrentJobs}
    };
}

size_t SimulationServer::GetQueueDepth() const {
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    return m_JobQueue.size();
}

void SimulationServer::WorkerThreadMain() {
    while (m_IsRunning) {
        SimulationJob job;
        
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_JobAvailable.wait(lock, [this] { return !m_JobQueue.empty() || !m_IsRunning; });
            
            if (!m_IsRunning) break;
            if (m_JobQueue.empty()) continue;
            
            job = m_JobQueue.front();
            m_JobQueue.pop();
        }

        {
            std::lock_guard<std::mutex> lock(m_HistoryMutex);
            auto historyIt = m_JobHistory.find(job.jobId);
            if (historyIt != m_JobHistory.end() && historyIt->second.status == JobStatus::Cancelled) {
                continue;
            }
        }

        job.status = JobStatus::Running;
        job.startedTime = std::chrono::system_clock::now().time_since_epoch().count();
        {
            std::lock_guard<std::mutex> lock(m_HistoryMutex);
            m_JobHistory[job.jobId] = job;
        }

        // Execute simulation
        if (ExecuteSimulationJob(job)) {
            job.status = JobStatus::Completed;
        } else {
            job.status = JobStatus::Failed;
        }

        job.completedTime = std::chrono::system_clock::now().time_since_epoch().count();

        // Store result
        {
            std::lock_guard<std::mutex> lock(m_HistoryMutex);
            m_JobHistory[job.jobId] = job;
        }

        BroadcastJobUpdate(job);
    }
}

bool SimulationServer::ExecuteSimulationJob(SimulationJob& job) {
    // TODO: Route to appropriate simulation module
    // - "stress" -> FEA Linear Static Solver
    // - "thermal" -> Thermal Engine
    // - "modal" -> Modal Analysis
    // - "optimization" -> AI Service

    job.progress = 0.5f;
    job.result = {
        {"simulationType", job.simulationType},
        {"componentId", job.componentId},
        {"completed", true}
    };

    return true;
}

std::string SimulationServer::GenerateJobId() {
    return MakeJobId();
}

bool SimulationServer::ValidateJobParameters(const SimulationJob& job) {
    // TODO: Validate job parameters
    return !job.simulationType.empty() && job.componentId > 0;
}

void SimulationServer::BroadcastJobUpdate(const SimulationJob& job) {
    // TODO: Broadcast update via WebSocket to connected clients
}

bool SimulationServer::ExpiredJob(const SimulationJob& job) const {
    // TODO: Check if job exceeded timeout
    return false;
}

}
