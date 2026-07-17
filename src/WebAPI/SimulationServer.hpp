#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>

namespace SZM::WebAPI {

    using json = nlohmann::json;

    /**
     * @brief Enhanced simulation server with FastAPI integration
     * Features:
     * - Async job queuing and execution
     * - Real-time progress updates via WebSocket
     * - Material property API
     * - Component analysis endpoints
     * - Results caching and history
     */
    class SimulationServer {
    public:
        enum class JobStatus {
            Pending = 0,
            Running = 1,
            Completed = 2,
            Failed = 3,
            Cancelled = 4
        };

        struct SimulationJob {
            std::string jobId;
            std::string simulationType;  // "stress", "thermal", "modal", "optimization"
            uint32_t componentId = 0;
            json parameters;
            JobStatus status = JobStatus::Pending;
            float progress = 0.0f;
            json result;
            std::string errorMessage;
            uint64_t createdTime = 0;
            uint64_t startedTime = 0;
            uint64_t completedTime = 0;
            std::string priority = "normal";  // high, normal, low
        };

        struct ServerConfig {
            uint16_t port = 8000;
            std::string host = "0.0.0.0";
            uint32_t maxConcurrentJobs = 4;
            uint32_t maxQueuedJobs = 100;
            bool enableWebSocket = true;
            bool enableCORS = true;
            std::string corsOrigin = "*";
            uint32_t jobTimeoutSeconds = 3600;
            uint32_t resultCacheMaxItems = 1000;
            bool enableResultCache = true;
        };

        SimulationServer();
        explicit SimulationServer(const ServerConfig& config);
        ~SimulationServer();

        /**
         * @brief Start the REST API server
         * Spawns worker threads and listens for connections
         */
        bool Start();

        /**
         * @brief Stop the server gracefully
         */
        void Stop();

        bool IsRunning() const { return m_IsRunning; }

        // ===== Simulation Job Management =====

        /**
         * @brief Submit a simulation job
         * @param job Job parameters
         * @return Job ID (UUID-like string)
         */
        std::string SubmitJob(const SimulationJob& job);

        /**
         * @brief Cancel a queued or running job
         */
        bool CancelJob(const std::string& jobId);

        /**
         * @brief Get job status and progress
         */
        SimulationJob GetJobStatus(const std::string& jobId);

        /**
         * @brief Get all jobs for a component
         */
        std::vector<SimulationJob> GetComponentJobs(uint32_t componentId);

        /**
         * @brief Get job history/results
         * @param limit Maximum results to return
         * @param offset Pagination offset
         */
        std::vector<SimulationJob> GetJobHistory(size_t limit = 50, size_t offset = 0);

        // ===== Material Database API =====

        /**
         * @brief Get available materials
         * @param filter Optional filter (e.g., "steel", "aluminum")
         */
        json GetMaterials(const std::string& filter = "");

        /**
         * @brief Get material properties
         */
        json GetMaterialProperties(const std::string& materialName);

        /**
         * @brief Update material properties
         */
        bool UpdateMaterialProperties(const std::string& materialName, const json& properties);

        // ===== Component Analysis =====

        /**
         * @brief Get component info (geometry, properties, assembly)
         */
        json GetComponentInfo(uint32_t componentId);

        /**
         * @brief Run interference check
         */
        json CheckInterference(uint32_t componentId1, uint32_t componentId2);

        /**
         * @brief Calculate mass properties (inertia, COM, etc)
         */
        json CalculateMassProperties(uint32_t componentId);

        // ===== Results Management =====

        /**
         * @brief Get cached result for job
         */
        json GetJobResult(const std::string& jobId);

        /**
         * @brief Stream results as they complete (WebSocket friendly)
         */
        bool StreamJobResult(const std::string& jobId, 
                           std::function<void(const json&)> callback);

        /**
         * @brief Clear result cache
         */
        void ClearResultCache();

        // ===== Configuration =====

        void SetConfig(const ServerConfig& config) { m_Config = config; }
        ServerConfig GetConfig() const { return m_Config; }

        /**
         * @brief Get server statistics
         */
        json GetServerStats();

        /**
         * @brief Get queue depth
         */
        size_t GetQueueDepth() const;

    private:
        ServerConfig m_Config;
        bool m_IsRunning = false;
        std::queue<SimulationJob> m_JobQueue;
        std::map<std::string, SimulationJob> m_JobHistory;
        std::map<std::string, json> m_ResultCache;
        mutable std::mutex m_QueueMutex;
        std::mutex m_HistoryMutex;
        std::mutex m_CacheMutex;
        std::condition_variable m_JobAvailable;
        std::vector<std::thread> m_WorkerThreads;

        // Worker thread function
        void WorkerThreadMain();

        // Job execution
        bool ExecuteSimulationJob(SimulationJob& job);

        // Helper methods
        std::string GenerateJobId();
        bool ValidateJobParameters(const SimulationJob& job);
        void BroadcastJobUpdate(const SimulationJob& job);
        bool ExpiredJob(const SimulationJob& job) const;
    };

}
