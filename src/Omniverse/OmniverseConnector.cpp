#include "OmniverseConnector.hpp"
#include <iostream>
#include <chrono>
#include <fstream>

namespace SZM::Omniverse {

void OmniverseConnector::Initialize(const OmniverseConfig& config) {
    m_config = config;
    if (m_initialized) return;
    
    std::cout << "[Omniverse] Initializing connector to Nucleus server: " << m_config.nucleusServer << std::endl;
    // In a full implementation, we would call omniClientInitialize() here
    
    // Start our async networking loop (mimicking ovstorage)
    m_workerRunning = true;
    m_workerThread = std::thread(&OmniverseConnector::WorkerLoop, this);
    
    m_initialized = true;
}

void OmniverseConnector::Shutdown() {
    if (!m_initialized) return;
    
    std::cout << "[Omniverse] Shutting down connector." << std::endl;
    
    // Stop worker thread
    m_workerRunning = false;
    m_queueCv.notify_all();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    
    // omniClientShutdown();
    
    m_initialized = false;
    m_connected = false;
}

bool OmniverseConnector::IsConnected() const {
    return m_connected;
}

bool OmniverseConnector::PushAssetToNucleus(const std::string& localUsdPath, const std::string& remoteFileName) {
    if (!m_connected) {
        std::cerr << "[Omniverse] Cannot push asset. Not connected to Nucleus." << std::endl;
        return false;
    }
    
    std::string remoteUrl = "omniverse://" + m_config.nucleusServer + m_config.basePath + "/" + remoteFileName;
    std::cout << "[Omniverse] Queuing file asset " << localUsdPath << " for upload to " << remoteUrl << std::endl;
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_uploadQueue.push({remoteFileName, {}, false, localUsdPath});
    }
    m_queueCv.notify_one();
    
    return true;
}

bool OmniverseConnector::PushAssetStream(const std::string& remoteFileName, const std::vector<uint8_t>& data) {
    if (!m_connected) {
        std::cerr << "[Omniverse] Cannot push stream. Not connected to Nucleus." << std::endl;
        return false;
    }
    
    std::string remoteUrl = "omniverse://" + m_config.nucleusServer + m_config.basePath + "/" + remoteFileName;
    std::cout << "[Omniverse] Queuing stream asset (" << data.size() << " bytes) for upload to " << remoteUrl << std::endl;
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_uploadQueue.push({remoteFileName, data, true, ""});
    }
    m_queueCv.notify_one();
    
    return true;
}

void OmniverseConnector::RegisterSyncCallback(const std::string& id, SyncCallback callback) {
    // Register callback for live sync
    // In a full implementation, we would register Omniverse Client library callbacks
    std::cout << "[Omniverse] Registered live sync callback for: " << id << std::endl;
}

void OmniverseConnector::PerformHandshake() {
    std::cout << "[Omniverse] Performing authentication handshake with " << m_config.nucleusServer << "..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    m_authToken = "simulated_token_xyz_123";
    m_connected = true;
    std::cout << "[Omniverse] Connection established. Token: " << m_authToken << std::endl;
}

bool OmniverseConnector::TransmitBuffer(const std::string& url, const std::vector<uint8_t>& buffer) {
    std::cout << "[Omniverse] Transmitting " << buffer.size() << " bytes to " << url << "..." << std::endl;
    // Simulate network latency
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "[Omniverse] Transmission complete for " << url << std::endl;
    return true;
}

void OmniverseConnector::WorkerLoop() {
    // 1. Initial Handshake
    PerformHandshake();
    
    while (m_workerRunning) {
        UploadTask task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCv.wait(lock, [this] { return !m_uploadQueue.empty() || !m_workerRunning; });
            
            if (!m_workerRunning && m_uploadQueue.empty()) {
                break;
            }
            
            task = m_uploadQueue.front();
            m_uploadQueue.pop();
        }
        
        std::string remoteUrl = "omniverse://" + m_config.nucleusServer + m_config.basePath + "/" + task.remoteFileName;
        
        if (task.isStream) {
            TransmitBuffer(remoteUrl, task.data);
        } else {
            // Read file into buffer and transmit
            std::ifstream file(task.localFilePath, std::ios::binary | std::ios::ate);
            if (file) {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<uint8_t> buffer(size);
                if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                    TransmitBuffer(remoteUrl, buffer);
                } else {
                    std::cerr << "[Omniverse] Failed to read file: " << task.localFilePath << std::endl;
                }
            } else {
                std::cerr << "[Omniverse] File not found: " << task.localFilePath << std::endl;
            }
        }
    }
}

} // namespace SZM::Omniverse
