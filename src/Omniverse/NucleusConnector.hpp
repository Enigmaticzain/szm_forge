#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace SZM::Omniverse {

/**
 * @class NucleusConnector
 * @brief Wrapper for NVIDIA Omniverse Nucleus API
 * 
 * Handles connection, authentication, and scene operations
 */
class NucleusConnector {
public:
    /**
     * @brief Get singleton instance
     */
    static NucleusConnector& GetInstance();
    
    // ===== Connection =====
    
    /**
     * @brief Connect to Nucleus server
     * @param nucleus_url Server URL (e.g., http://localhost:3100)
     * @param api_key Optional API key for authentication
     * @return true if connection successful
     */
    bool Connect(const std::string& nucleus_url, const std::string& api_key = "");
    
    /**
     * @brief Disconnect from Nucleus
     * @return true if successful
     */
    bool Disconnect();
    
    /**
     * @brief Check connection status
     * @return true if connected
     */
    bool IsConnected() const;
    
    // ===== Authentication =====
    
    /**
     * @brief Set credentials for authentication
     * @param username Nucleus username
     * @param password Nucleus password
     */
    void SetCredentials(const std::string& username, const std::string& password);
    
    /**
     * @brief Authenticate with Nucleus server
     * @return true if authentication successful
     */
    bool Authenticate();
    
    // ===== Scene Operations =====
    
    /**
     * @brief Open scene from Nucleus
     * @param project_path Project path
     * @param scene_file Scene file name
     * @return Opaque scene handle (pointer)
     */
    void* OpenScene(const std::string& project_path, const std::string& scene_file);
    
    /**
     * @brief Save scene to Nucleus
     * @param scene_handle Scene to save
     * @return true if successful
     */
    bool SaveScene(void* scene_handle);
    
    /**
     * @brief Delete scene from Nucleus
     * @param project_path Project path
     * @return true if successful
     */
    bool DeleteScene(const std::string& project_path);
    
    // ===== Layer Management =====
    
    /**
     * @brief Get layers in project
     * @param project_path Project path
     * @return Vector of layer names
     */
    std::vector<std::string> GetLayers(const std::string& project_path);
    
    /**
     * @brief Get specific layer
     * @param layer_path Layer path
     * @return Opaque layer handle
     */
    void* GetLayer(const std::string& layer_path);
    
    /**
     * @brief Mute layer (hide from view)
     * @param layer_path Layer to mute
     * @return true if successful
     */
    bool MuteLayer(const std::string& layer_path);
    
    /**
     * @brief Unmute layer
     * @param layer_path Layer to unmute
     * @return true if successful
     */
    bool UnmuteLayer(const std::string& layer_path);
    
    // ===== Versioning =====
    
    /**
     * @brief Get available versions
     * @param scene_path Scene path
     * @return Vector of version IDs
     */
    std::vector<std::string> GetVersions(const std::string& scene_path);
    
    /**
     * @brief Create checkpoint/version
     * @param scene_path Scene path
     * @param comment Version comment
     * @return Checkpoint ID
     */
    std::string CreateCheckpoint(
        const std::string& scene_path,
        const std::string& comment);
    
    /**
     * @brief Revert to checkpoint
     * @param checkpoint_id Checkpoint to revert to
     * @return true if successful
     */
    bool RevertToCheckpoint(const std::string& checkpoint_id);
    
    // ===== Metadata =====
    
    /**
     * @brief Get project metadata
     * @param project_path Project path
     * @return Metadata JSON string
     */
    std::string GetProjectMetadata(const std::string& project_path);
    
    /**
     * @brief Set project metadata
     * @param project_path Project path
     * @param metadata Metadata JSON string
     * @return true if successful
     */
    bool SetProjectMetadata(const std::string& project_path, const std::string& metadata);

private:
    // Private constructor (singleton)
    NucleusConnector();
    ~NucleusConnector();
    
    // Delete copy/move operations
    NucleusConnector(const NucleusConnector&) = delete;
    NucleusConnector& operator=(const NucleusConnector&) = delete;
    
    // Connection state
    bool is_connected_;
    std::string nucleus_url_;
    std::string username_;
    std::string password_;
    std::string api_key_;
    
    // Opaque Python Nucleus client instance
    void* nucleus_client_;
};

} // namespace SZM::Omniverse
