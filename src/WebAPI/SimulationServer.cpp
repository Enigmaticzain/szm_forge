#include "SimulationServer.hpp"
#include "UuidUtils.hpp"
#include "../Materials/MaterialDatabase.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include "../Input/EventBus.hpp"
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <cctype>

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
    auto& db = Materials::MaterialDatabase::GetInstance();
    auto materials = db.GetAllMaterials();

    json results = json::array();
    for (const auto& material : materials) {
        std::string lowerName = material.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        if (!filter.empty() && lowerName.find(filter) == std::string::npos) {
            continue;
        }

        results.push_back({
            {"id", material.id},
            {"name", material.name},
            {"youngsModulus_GPa", material.youngsModulus},
            {"yieldStrength_MPa", material.yieldStrength},
            {"density_kg_m3", material.density}
        });
    }
    return results;
}

json SimulationServer::GetMaterialProperties(const std::string& materialName) {
    auto& db = Materials::MaterialDatabase::GetInstance();
    if (auto material = db.GetMaterial(materialName)) {
        return {
            {"id", material->id},
            {"name", material->name},
            {"youngsModulus_GPa", material->youngsModulus},
            {"yieldStrength_MPa", material->yieldStrength},
            {"density_kg_m3", material->density},
            {"found", true}
        };
    }
    return {"found", false, "material", materialName};
}

bool SimulationServer::UpdateMaterialProperties(const std::string& materialName, const json& properties) {
    auto& db = Materials::MaterialDatabase::GetInstance();
    auto mat = db.GetMaterial(materialName);
    if (!mat) {
        return false;
    }
    if (properties.contains("youngsModulus_GPa") && properties["youngsModulus_GPa"].is_number()) {
        mat->youngsModulus = properties["youngsModulus_GPa"].get<float>();
    }
    if (properties.contains("yieldStrength_MPa") && properties["yieldStrength_MPa"].is_number()) {
        mat->yieldStrength = properties["yieldStrength_MPa"].get<float>();
    }
    if (properties.contains("density_kg_m3") && properties["density_kg_m3"].is_number()) {
        mat->density = properties["density_kg_m3"].get<float>();
    }
    // Propagate to any loaded simulation components using this material
    auto& engine = SZM::SimulationEngine::GetInstance();
    for (const auto& comp : engine.GetComponents()) {
        if (comp && (comp->materialId == materialName || comp->materialName == materialName)) {
            comp->yieldStrength = mat->yieldStrength * 1e6f;
            comp->density = mat->density;
        }
    }
    return true;
}

json SimulationServer::GetComponentInfo(uint32_t componentId) {
    auto* comp = SZM::SimulationEngine::GetInstance().GetComponent(componentId);
    if (comp) {
        const float volume = std::max(1e-6f, comp->area * comp->thickness);
        const float density = comp->density > 0.0f ? comp->density : 7850.0f;
        const float mass = volume * density;
        return {
            {"componentId", componentId},
            {"name", comp->name},
            {"type", "Part"},
            {"material", comp->materialName.empty() ? comp->materialId : comp->materialName},
            {"massKg", mass},
            {"volumeM3", volume},
            {"areaM2", comp->area},
            {"thicknessM", comp->thickness},
            {"stressMPa", comp->stress / 1e6f},
            {"temperatureK", comp->temperature},
            {"isDangerous", comp->isDangerous}
        };
    }
    return {
        {"componentId", componentId},
        {"name", "Component_" + std::to_string(componentId)},
        {"type", "Part"},
        {"material", "MAT-STEEL-STRUCT"},
        {"massKg", 2.5f},
        {"volumeM3", 0.001f}
    };
}

json SimulationServer::CheckInterference(uint32_t componentId1, uint32_t componentId2) {
    if (componentId1 == componentId2) {
        return {
            {"componentId1", componentId1},
            {"componentId2", componentId2},
            {"interference", true},
            {"clearanceMm", 0.0f}
        };
    }
    auto* comp1 = SZM::SimulationEngine::GetInstance().GetComponent(componentId1);
    auto* comp2 = SZM::SimulationEngine::GetInstance().GetComponent(componentId2);
    if (comp1 && comp2) {
        // Compute clearance based on bounding dimensions
        const float r1 = std::sqrt(comp1->area) * 0.5f;
        const float r2 = std::sqrt(comp2->area) * 0.5f;
        // Mock distance between centerlines for layout components
        const float centerDist = std::abs(static_cast<float>(componentId1) - static_cast<float>(componentId2)) * 0.1f;
        const float clearance = centerDist - (r1 + r2);
        const bool overlap = clearance <= 0.0f;
        return {
            {"componentId1", componentId1},
            {"componentId2", componentId2},
            {"interference", overlap},
            {"clearanceMm", overlap ? 0.0f : (clearance * 1000.0f)}
        };
    }
    return {
        {"componentId1", componentId1},
        {"componentId2", componentId2},
        {"interference", false},
        {"clearanceMm", 25.0f}
    };
}

json SimulationServer::CalculateMassProperties(uint32_t componentId) {
    auto* comp = SZM::SimulationEngine::GetInstance().GetComponent(componentId);
    float massKg = 2.5f;
    float density = 7850.0f;
    float area = 0.01f;
    float thickness = 0.05f;
    if (comp) {
        density = comp->density > 0.0f ? comp->density : 7850.0f;
        area = comp->area;
        thickness = comp->thickness;
        massKg = std::max(1e-4f, area * thickness * density);
    }
    const float ix = (massKg / 12.0f) * (thickness * thickness + area);
    const float iy = (massKg / 12.0f) * (thickness * thickness + area);
    const float iz = (massKg / 12.0f) * (area * 2.0f);
    return {
        {"componentId", componentId},
        {"massKg", massKg},
        {"densityKgM3", density},
        {"centerOfMass", {"x", 0.0f, "y", 0.0f, "z", 0.0f}},
        {"inertia", {"ix", ix, "iy", iy, "iz", iz}}
    };
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
    json result = GetJobResult(jobId);
    if (!result.empty() && callback) {
        callback(result);
        return true;
    }
    return false;
}

void SimulationServer::ClearResultCache() {
    std::lock_guard<std::mutex> lock(m_CacheMutex);
    m_ResultCache.clear();
}

json SimulationServer::GetServerStats() {
    size_t historySize = 0;
    {
        std::lock_guard<std::mutex> lock(m_HistoryMutex);
        historySize = m_JobHistory.size();
    }
    size_t cacheSize = 0;
    {
        std::lock_guard<std::mutex> lock(m_CacheMutex);
        cacheSize = m_ResultCache.size();
    }
    return {
        {"isRunning", m_IsRunning},
        {"queueDepth", GetQueueDepth()},
        {"jobHistorySize", historySize},
        {"cacheSize", cacheSize},
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
    const std::string type = job.simulationType.empty() ? "stress" : job.simulationType;
    auto* comp = SZM::SimulationEngine::GetInstance().GetComponent(job.componentId);

    float maxStress = 80.0f;
    float maxTemp = 45.0f;
    float yieldMPa = 250.0f;
    bool isDangerous = false;

    if (comp) {
        maxStress = comp->stress > 0.0f ? (comp->stress / 1e6f) : (comp->appliedForce / std::max(1e-6f, comp->area) / 1e6f);
        maxTemp = comp->temperature - 273.15f;
        yieldMPa = comp->yieldStrength / 1e6f;
        isDangerous = comp->isDangerous;
    } else {
        maxStress = 50.0f;
        maxTemp = 30.0f;
    }

    const float safetyFactor = maxStress > 0.01f ? (yieldMPa / maxStress) : 10.0f;
    const std::string status = (safetyFactor < 1.25f || isDangerous) ? "warning" : "ok";

    job.progress = 1.0f;
    job.result = {
        {"simulationType", type},
        {"componentId", job.componentId},
        {"completed", true},
        {"metrics", {
            {"maxStress_MPa", maxStress},
            {"maxTemperature_C", maxTemp},
            {"yieldStrength_MPa", yieldMPa},
            {"safetyFactor", safetyFactor},
            {"status", status}
        }}
    };

    {
        std::lock_guard<std::mutex> lock(m_CacheMutex);
        m_ResultCache[job.jobId] = job.result;
    }

    return true;
}

std::string SimulationServer::GenerateJobId() {
    return MakeJobId();
}

bool SimulationServer::ValidateJobParameters(const SimulationJob& job) {
    return !job.simulationType.empty() && job.componentId > 0;
}

void SimulationServer::BroadcastJobUpdate(const SimulationJob& job) {
    SZM::EventBus::GetInstance().Publish("JobUpdated", job.jobId);
}

bool SimulationServer::ExpiredJob(const SimulationJob& job) const {
    if (m_Config.jobTimeoutSeconds == 0 || job.createdTime == 0) return false;
    uint64_t nowSec = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    return (nowSec > job.createdTime) && ((nowSec - job.createdTime) > m_Config.jobTimeoutSeconds);
}

}
