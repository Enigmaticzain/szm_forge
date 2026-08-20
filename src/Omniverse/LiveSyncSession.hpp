#pragma once

#include "OmniverseLiveSync.hpp"
#include <string>
#include <vector>
#include <mutex>

namespace SZM::Omniverse {

/**
 * @class LiveSyncSession
 * @brief Manages individual collaborative session state
 * 
 * Tracks user connections, pending changes, and conflicts
 */
class LiveSyncSession {
public:
    /**
     * @brief Constructor
     * @param session_id Unique session identifier
     */
    explicit LiveSyncSession(const std::string& session_id);
    
    // ===== Session Lifecycle =====
    
    /**
     * @brief Start session
     */
    void Start();
    
    /**
     * @brief Stop session
     */
    void Stop();
    
    /**
     * @brief Check if session is active
     * @return true if running
     */
    bool IsActive() const;
    
    // ===== User Management =====
    
    /**
     * @brief Add user to session
     * @param user_id User to add
     */
    void AddUser(const std::string& user_id);
    
    /**
     * @brief Remove user from session
     * @param user_id User to remove
     */
    void RemoveUser(const std::string& user_id);
    
    /**
     * @brief Get connected users
     * @return Vector of user IDs
     */
    std::vector<std::string> GetConnectedUsers() const;
    
    // ===== Change Tracking =====
    
    /**
     * @brief Record a change
     * @param change Change to record
     */
    void RecordChange(const SceneChange& change);
    
    /**
     * @brief Get pending changes
     * @return Vector of changes
     */
    std::vector<SceneChange> GetPendingChanges();
    
    /**
     * @brief Clear pending changes
     */
    void ClearPendingChanges();
    
    /**
     * @brief Get change history
     * @return Vector of all changes
     */
    std::vector<SceneChange> GetChangeHistory() const;
    
    // ===== State Synchronization =====
    
    /**
     * @brief Perform synchronization
     */
    void SyncState();
    
    /**
     * @brief Apply remote changes
     * @param changes Changes from remote
     */
    void ApplyRemoteChanges(const std::vector<SceneChange>& changes);
    
    // ===== Conflict Detection =====
    
    /**
     * @brief Check if session has unresolved conflicts
     * @return true if conflicts exist
     */
    bool HasConflicts() const;
    
    /**
     * @brief Get unresolved conflicts
     * @return Vector of conflicts
     */
    std::vector<ConflictInfo> GetConflicts() const;
    
    /**
     * @brief Resolve a conflict
     * @param change Change to apply
     * @param strategy Merge strategy
     */
    void ResolveConflict(const SceneChange& change, MergeStrategy strategy);
    
    // ===== Getters =====
    
    /**
     * @brief Get session ID
     */
    std::string GetSessionId() const;
    
    /**
     * @brief Get network latency
     * @return Latency in milliseconds
     */
    double GetLatency() const;
    
    /**
     * @brief Get pending change count
     */
    int GetChangeCount() const;
    
    /**
     * @brief Get session state
     */
    SessionState GetState() const;

private:
    std::string session_id_;
    std::vector<std::string> connected_users_;
    std::vector<SceneChange> pending_changes_;
    std::vector<SceneChange> change_history_;
    std::vector<ConflictInfo> conflicts_;
    double latency_ms_;
    int change_count_;
    SessionState state_;
    bool is_active_;
    mutable std::mutex mutex_;
};

} // namespace SZM::Omniverse
