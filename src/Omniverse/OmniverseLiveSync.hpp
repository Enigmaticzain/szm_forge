#pragma once

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <functional>
#include <complex>

namespace SZM::Omniverse {

// ============= Enums =============

/**
 * @brief Session connection states
 */
enum class SessionState {
    DISCONNECTED = 0,    ///< Not connected
    CONNECTING = 1,      ///< Connection in progress
    CONNECTED = 2,       ///< Successfully connected
    SYNCING = 3,         ///< Synchronization in progress
    CONFLICT = 4,        ///< Conflict detected
    ERROR = 5            ///< Error state
};

/**
 * @brief Types of scene modifications
 */
enum class ChangeType {
    PROPERTY_CHANGED = 0,      ///< Property value modified
    PRIM_CREATED = 1,          ///< New primitive created
    PRIM_DELETED = 2,          ///< Primitive deleted
    RELATIONSHIP_CHANGED = 3,  ///< Relationship modified
    METADATA_CHANGED = 4,      ///< Metadata updated
    LAYER_MUTED = 5,           ///< Layer muted
    LAYER_UNMUTED = 6          ///< Layer unmuted
};

/**
 * @brief Conflict resolution strategies
 */
enum class MergeStrategy {
    LAST_WRITE_WINS = 0,    ///< Accept remote version
    KEEP_LOCAL = 1,         ///< Keep local version
    MERGE_PROPERTIES = 2,   ///< Merge non-overlapping properties
    MANUAL = 3              ///< User decides via UI
};

// ============= Structures =============

/**
 * @brief Single scene change/modification
 */
struct SceneChange {
    ChangeType type;                ///< Type of change
    std::string prim_path;          ///< USD prim path (e.g., /Group/Cube)
    std::string property_name;      ///< Property name (optional)
    std::string old_value;          ///< Previous value
    std::string new_value;          ///< New value
    double timestamp;               ///< Unix timestamp
    std::string user_id;            ///< User who made change
    std::string session_id;         ///< Session ID
    
    SceneChange() : type(ChangeType::PROPERTY_CHANGED), timestamp(0.0) {}
};

/**
 * @brief User information in session
 */
struct UserInfo {
    std::string user_id;            ///< Unique user identifier
    std::string display_name;       ///< User display name
    double last_activity_time;      ///< Last activity timestamp
    bool is_active;                 ///< Currently connected
    
    UserInfo() : last_activity_time(0.0), is_active(true) {}
};

/**
 * @brief Complete session metadata
 */
struct OmniverseSessionInfo {
    std::string session_id;                     ///< Unique session ID
    std::string nucleus_url;                    ///< Nucleus server URL
    std::string project_path;                   ///< Nucleus project path
    std::string scene_file;                     ///< Scene file name
    SessionState state;                         ///< Current session state
    std::vector<std::string> connected_users;   ///< Connected user IDs
    int change_count;                           ///< Number of pending changes
    int total_changes;                          ///< Total changes in history
    double sync_latency_ms;                     ///< Estimated sync latency
    std::string created_at;                     ///< Session creation time
    std::string last_sync;                      ///< Last synchronization time
    int conflict_count;                         ///< Unresolved conflicts
    
    OmniverseSessionInfo()
        : state(SessionState::DISCONNECTED),
          change_count(0),
          total_changes(0),
          sync_latency_ms(0.0),
          conflict_count(0) {}
};

/**
 * @brief Detected conflict requiring resolution
 */
struct ConflictInfo {
    std::string conflict_id;                    ///< Unique conflict ID
    SceneChange local_change;                   ///< Local modification
    SceneChange remote_change;                  ///< Remote modification
    std::string prim_path;                      ///< Conflicted prim
    double timestamp;                           ///< Conflict detection time
    
    ConflictInfo() : timestamp(0.0) {}
};

// ============= Main Orchestrator Class =============

/**
 * @class OmniverseLiveSync
 * @brief Main class for real-time collaborative Omniverse scene editing
 * 
 * Manages sessions, change synchronization, and conflict resolution
 * Singleton pattern for single instance
 */
class OmniverseLiveSync {
public:
    /**
     * @brief Get singleton instance
     */
    static OmniverseLiveSync& GetInstance();
    
    // ===== Session Management =====
    
    /**
     * @brief Create new collaborative session
     * @param nucleus_url Nucleus server URL
     * @param project_path Project path on Nucleus
     * @param scene_file Scene file to open
     * @return Session ID
     */
    std::string CreateSession(
        const std::string& nucleus_url,
        const std::string& project_path,
        const std::string& scene_file);
    
    /**
     * @brief User joins existing session
     * @param session_id Session to join
     * @param user_id User identifier
     * @return true if successful
     */
    bool JoinSession(
        const std::string& session_id,
        const std::string& user_id);
    
    /**
     * @brief User leaves session
     * @param session_id Session ID
     * @param user_id User leaving
     * @return true if successful
     */
    bool LeaveSession(
        const std::string& session_id,
        const std::string& user_id);
    
    /**
     * @brief Close session (removes all users)
     * @param session_id Session to close
     */
    void CloseSession(const std::string& session_id);
    
    // ===== Scene Operations =====
    
    /**
     * @brief Open remote scene from Nucleus
     * @param session_id Session ID
     * @return true if successful
     */
    bool OpenRemoteScene(const std::string& session_id);
    
    /**
     * @brief Save local changes to Nucleus
     * @param session_id Session ID
     * @return Number of changes saved
     */
    int SaveLocalChanges(const std::string& session_id);
    
    /**
     * @brief Push local changes to Nucleus
     * @param session_id Session ID
     * @param changes Changes to push
     * @return Number of changes accepted
     */
    int PushChanges(
        const std::string& session_id,
        const std::vector<SceneChange>& changes);
    
    /**
     * @brief Pull remote changes from Nucleus
     * @param session_id Session ID
     * @return Vector of remote changes
     */
    std::vector<SceneChange> PullChanges(const std::string& session_id);
    
    // ===== Conflict Handling =====
    
    /**
     * @brief Resolve conflict with merge strategy
     * @param session_id Session ID
     * @param conflict Conflict to resolve
     * @param strategy Merge strategy
     * @return true if resolved
     */
    bool ResolveConflict(
        const std::string& session_id,
        const ConflictInfo& conflict,
        MergeStrategy strategy);
    
    /**
     * @brief Get unresolved conflicts
     * @param session_id Session ID
     * @return Vector of conflicts
     */
    std::vector<ConflictInfo> GetConflicts(const std::string& session_id);
    
    // ===== Subscriptions =====
    
    /**
     * @brief Subscribe to change notifications
     * @param session_id Session ID
     * @param callback Function called for each change
     */
    void SubscribeToChanges(
        const std::string& session_id,
        std::function<void(const SceneChange&)> callback);
    
    /**
     * @brief Unsubscribe from changes
     * @param session_id Session ID
     */
    void UnsubscribeFromChanges(const std::string& session_id);
    
    // ===== State Queries =====
    
    /**
     * @brief Get session state
     * @param session_id Session ID
     * @return Current session state
     */
    SessionState GetSessionState(const std::string& session_id);
    
    /**
     * @brief Get complete session info
     * @param session_id Session ID
     * @return Session information
     */
    OmniverseSessionInfo GetSessionInfo(const std::string& session_id);
    
    /**
     * @brief List all active sessions
     * @return Vector of session IDs
     */
    std::vector<std::string> ListActiveSessions();
    
    /**
     * @brief Get connected users in session
     * @param session_id Session ID
     * @return Vector of user IDs
     */
    std::vector<std::string> GetConnectedUsers(const std::string& session_id);
    
    // ===== Configuration =====
    
    /**
     * @brief Set default merge strategy
     * @param strategy Merge strategy
     */
    void SetMergeStrategy(MergeStrategy strategy);
    
    /**
     * @brief Enable/disable automatic sync
     * @param enabled true to enable
     */
    void EnableAutoSync(bool enabled);
    
    /**
     * @brief Set synchronization interval
     * @param seconds Interval in seconds
     */
    void SetSyncInterval(double seconds);
    
    /**
     * @brief Force immediate synchronization
     * @param session_id Session ID
     * @return true if successful
     */
    bool ForceSync(const std::string& session_id);

private:
    // Private constructor (singleton)
    OmniverseLiveSync();
    ~OmniverseLiveSync();
    
    // Delete copy/move operations
    OmniverseLiveSync(const OmniverseLiveSync&) = delete;
    OmniverseLiveSync& operator=(const OmniverseLiveSync&) = delete;
    
    // ===== Internal Session Data =====
    
    struct SessionData {
        OmniverseSessionInfo info;
        std::vector<SceneChange> pending_changes;
        std::vector<SceneChange> change_history;
        std::vector<ConflictInfo> conflicts;
        std::queue<SceneChange> incoming_changes;
        std::function<void(const SceneChange&)> change_callback;
        bool auto_sync;
        double sync_interval_s;
    };
    
    std::map<std::string, SessionData> sessions_;
    MergeStrategy default_merge_strategy_;
    
    // ===== Private Methods =====
    
    /**
     * @brief Background sync thread for session
     */
    void SyncThread(const std::string& session_id);
    
    /**
     * @brief Process incoming remote changes
     */
    void ProcessIncomingChanges(const std::string& session_id);
    
    /**
     * @brief Detect conflicts in change sets
     */
    void DetectConflicts(const std::string& session_id);
    
    /**
     * @brief Apply merge strategy to conflict
     */
    SceneChange ApplyMergeStrategy(
        const ConflictInfo& conflict,
        MergeStrategy strategy);
};

} // namespace SZM::Omniverse
