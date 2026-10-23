#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>

namespace SZM::Omniverse {

struct OmniverseConfig {
    std::string nucleusServer = "localhost";
    std::string username = "omniverse";
    std::string password = "";
    std::string basePath = "/Projects/SZM_Forge";
};

class OmniverseConnector {
public:
    static OmniverseConnector& GetInstance() {
        static OmniverseConnector instance;
        return instance;
    }

    void Initialize(const OmniverseConfig& config = OmniverseConfig());
    void Shutdown();

    bool IsConnected() const;

    // Push a generated USD machine/asset to the Nucleus server async
    bool PushAssetToNucleus(const std::string& localUsdPath, const std::string& remoteFileName);
    
    // Push raw byte buffer (used when bypassing disk I/O)
    bool PushAssetStream(const std::string& remoteFileName, const std::vector<uint8_t>& data);

    // Live sync callbacks for when simulation updates parameters
    using SyncCallback = std::function<void(const std::string& path, const std::string& property, float value)>;
    void RegisterSyncCallback(const std::string& id, SyncCallback callback);

private:
    OmniverseConnector() = default;
    ~OmniverseConnector() { Shutdown(); }

    OmniverseConnector(const OmniverseConnector&) = delete;
    OmniverseConnector& operator=(const OmniverseConnector&) = delete;

    bool m_initialized = false;
    bool m_connected = false;
    OmniverseConfig m_config;
    
    // Nucleus connection token (simulated)
    std::string m_authToken;
    
    // Async worker components mimicking ovstorage client
    struct UploadTask {
        std::string remoteFileName;
        std::vector<uint8_t> data;
        bool isStream = false;
        std::string localFilePath;
    };
    
    std::queue<UploadTask> m_uploadQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCv;
    std::thread m_workerThread;
    std::atomic<bool> m_workerRunning{false};
    
    void WorkerLoop();
    void PerformHandshake();
    bool TransmitBuffer(const std::string& url, const std::vector<uint8_t>& buffer);
};

} // namespace SZM::Omniverse
