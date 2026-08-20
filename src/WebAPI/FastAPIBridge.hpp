#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <thread>

namespace SZM::WebAPI {

    /**
     * @brief REST API server for remote simulation access
     * Built on FastAPI bridge for Python integration
     */
    class SimulationAPI {
    public:
        struct JobRequest {
            std::string jobId;
            uint32_t componentId = 0;
            std::string simulationType;  // "stress", "thermal", "modal"
            std::vector<float> parameters;
            std::string priority = "normal";
        };

        struct JobResult {
            std::string jobId;
            std::string status;  // "pending", "running", "completed", "failed"
            float progress = 0.0f;
            std::string resultJSON;
            std::string errorMessage;
        };

        SimulationAPI(uint16_t port = 8000);
        ~SimulationAPI();

        /**
         * @brief Start REST API server
         */
        bool Start();
        void Stop();
        bool IsRunning() const { return m_IsRunning; }

        /**
         * @brief Submit simulation job
         */
        std::string SubmitJob(const JobRequest& request);

        /**
         * @brief Get job status
         */
        JobResult GetJobStatus(const std::string& jobId);

        /**
         * @brief Cancel running job
         */
        bool CancelJob(const std::string& jobId);

        /**
         * @brief Get job result
         */
        std::string GetJobResult(const std::string& jobId);

        uint32_t GetQueuedJobs() const { return m_QueuedJobs; }
        uint32_t GetRunningJobs() const { return m_RunningJobs; }

    private:
        uint16_t m_Port;
        bool m_IsRunning = false;
        uint32_t m_QueuedJobs = 0;
        uint32_t m_RunningJobs = 0;
        std::unordered_map<std::string, JobResult> m_Jobs;
        std::vector<std::thread> m_JobThreads;
        std::mutex m_JobMutex;
        std::atomic<bool> m_Shutdown = false;
        void* m_FastAPIContext = nullptr;
    };

    /**
     * @brief Analysis endpoints for remote queries
     */
    class AnalysisAPI {
    public:
        AnalysisAPI(uint16_t port = 8001);

        bool Start();
        void Stop();

        /**
         * @brief Query component properties
         */
        std::string GetComponentInfo(uint32_t componentId);

        /**
         * @brief Query assembly structure
         */
        std::string GetAssemblyTree();

        /**
         * @brief Query simulation history
         */
        std::string GetSimulationHistory(uint32_t componentId, uint32_t limit = 100);

        /**
         * @brief Get AI predictions
         */
        std::string GetPredictions(uint32_t componentId, const std::string& predictionType);

    private:
        uint16_t m_Port;
        bool m_IsRunning = false;
        void* m_FastAPIContext = nullptr;
    };

    /**
     * @brief Export endpoints for file downloads
     */
    class ExportAPI {
    public:
        ExportAPI(uint16_t port = 8002);

        bool Start();
        void Stop();

        /**
         * @brief Export component to STEP
         */
        std::string ExportComponentSTEP(uint32_t componentId);

        /**
         * @brief Export simulation results to CSV
         */
        std::string ExportResultsCSV(const std::string& jobId);

        /**
         * @brief Export scene to Three.js JSON
         */
        std::string ExportSceneThreeJS();

        /**
         * @brief Export scene to Pixar USD (via Python Bridge)
         */
        std::string ExportSceneUSD(const std::string& outputPath = "/tmp/workspace_export.usda");

        /**
         * @brief Export report as PDF
         */
        std::string ExportReportPDF(const std::string& jobId);

    private:
        uint16_t m_Port;
        bool m_IsRunning = false;
        void* m_FastAPIContext = nullptr;
    };

    // ============================================================================
    // Shared telemetry snapshot
    // ============================================================================
    struct SimulationSnapshot {
        std::mutex mu;
        double simTime = 0.0;
        struct ComponentSnap {
            std::string name;
            float stress = 0.0f;
            float temperature = 0.0f;
            float stressRatio = 0.0f;
            float tempRatio = 0.0f;
            std::string status = "SAFE";
        };
        std::vector<ComponentSnap> components;
        std::unordered_map<std::string, size_t> indexByName;
    };

    /**
     * @brief Manages all API endpoints
     */
    class APIManager {
    public:
        static APIManager& GetInstance();

        bool Initialize(uint16_t basePort = 8000, const std::string& uiDistPath = "new ui/dist");
        void Shutdown();

        SimulationAPI& GetSimulationAPI() { return m_SimulationAPI; }
        AnalysisAPI& GetAnalysisAPI() { return m_AnalysisAPI; }
        ExportAPI& GetExportAPI() { return m_ExportAPI; }

        /**
         * @brief Get API documentation URL
         */
        std::string GetDocumentationURL() const;

        /**
         * @brief Enable/disable authentication
         */
        void SetAuthenticationRequired(bool required) { m_AuthRequired = required; }

        bool IsReady() const { return m_IsReady; }

        /**
         * @brief Push live simulation data to the REST API snapshot
         */
        void PushComponentSnapshot(const std::string& name,
                                    float stress, float temperature,
                                    float stressRatio, float tempRatio,
                                    double simTime);

    private:
        APIManager() : m_SimulationAPI(8000), m_AnalysisAPI(8001), m_ExportAPI(8002) {}
        ~APIManager() = default;

        SimulationAPI m_SimulationAPI;
        AnalysisAPI m_AnalysisAPI;
        ExportAPI m_ExportAPI;
        uint16_t m_Port = 8000;
        bool m_AuthRequired = false;
        bool m_IsReady = false;
    };

} // namespace SZM::WebAPI
