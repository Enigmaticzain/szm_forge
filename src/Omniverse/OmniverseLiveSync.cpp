#include "OmniverseLiveSync.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>

namespace SZM::Omniverse {

// ============= Singleton Instance =============

static std::mutex instance_mutex;
static OmniverseLiveSync* instance = nullptr;

OmniverseLiveSync& OmniverseLiveSync::GetInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex);
    if (!instance) {
        instance = new OmniverseLiveSync();
    }
    return *instance;
}

// ============= Constructor/Destructor =============

OmniverseLiveSync::OmniverseLiveSync()
    : default_merge_strategy_(MergeStrategy::LAST_WRITE_WINS) {
    std::cout << "[OmniverseLiveSync] Initialized" << std::endl;
}

OmniverseLiveSync::~OmniverseLiveSync() {
    // Close all sessions
    auto session_ids = ListActiveSessions();
    for (const auto& sid : session_ids) {
        CloseSession(sid);
    }
    std::cout << "[OmniverseLiveSync] Destroyed" << std::endl;
}

// ============= Session Management =============

std::string OmniverseLiveSync::CreateSession(
    const std::string& nucleus_url,
    const std::string& project_path,
    const std::string& scene_file) {
    
    static int session_counter = 0;
    std::string session_id = "session_" + std::to_string(++session_counter);
    
    SessionData session_data;
    session_data.info.session_id = session_id;
    session_data.info.nucleus_url = nucleus_url;
    session_data.info.project_path = project_path;
    session_data.info.scene_file = scene_file;
    session_data.info.state = SessionState::CONNECTING;
    session_data.info.change_count = 0;
    session_data.info.total_changes = 0;
    session_data.info.sync_latency_ms = 50.0;
    session_data.info.conflict_count = 0;
    session_data.auto_sync = true;
    session_data.sync_interval_s = 1.0;
    
    // Set timestamps
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    session_data.info.created_at = std::ctime(&time);
    session_data.info.last_sync = session_data.info.created_at;
    
    sessions_[session_id] = session_data;
    
    // Update state
    sessions_[session_id].info.state = SessionState::CONNECTED;
    
    std::cout << "[OmniverseLiveSync] Created session " << session_id 
              << " at " << nucleus_url << std::endl;
    
    return session_id;
}

bool OmniverseLiveSync::JoinSession(
    const std::string& session_id,
    const std::string& user_id) {
    
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        std::cerr << "[OmniverseLiveSync] Session not found: " << session_id << std::endl;
        return false;
    }
    
    auto& session = it->second;
    
    // Add user if not already present
    auto user_it = std::find(
        session.info.connected_users.begin(),
        session.info.connected_users.end(),
        user_id);
    
    if (user_it == session.info.connected_users.end()) {
        session.info.connected_users.push_back(user_id);
        std::cout << "[OmniverseLiveSync] User " << user_id 
                  << " joined session " << session_id << std::endl;
    }
    
    return true;
}

bool OmniverseLiveSync::LeaveSession(
    const std::string& session_id,
    const std::string& user_id) {
    
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }
    
    auto& session = it->second;
    
    // Remove user
    auto user_it = std::find(
        session.info.connected_users.begin(),
        session.info.connected_users.end(),
        user_id);
    
    if (user_it != session.info.connected_users.end()) {
        session.info.connected_users.erase(user_it);
        std::cout << "[OmniverseLiveSync] User " << user_id 
                  << " left session " << session_id << std::endl;
    }
    
    return true;
}

void OmniverseLiveSync::CloseSession(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        std::cout << "[OmniverseLiveSync] Closing session " << session_id << std::endl;
        sessions_.erase(it);
    }
}

// ============= Scene Operations =============

bool OmniverseLiveSync::OpenRemoteScene(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }
    
    auto& session = it->second;
    session.info.state = SessionState::SYNCING;
    
    std::cout << "[OmniverseLiveSync] Opening remote scene: " 
              << session.info.scene_file << std::endl;
    
    // Simulate scene load
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    session.info.state = SessionState::CONNECTED;
    return true;
}

int OmniverseLiveSync::SaveLocalChanges(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return 0;
    }
    
    auto& session = it->second;
    int count = session.pending_changes.size();
    
    if (count > 0) {
        session.change_history.insert(
            session.change_history.end(),
            session.pending_changes.begin(),
            session.pending_changes.end());
        session.pending_changes.clear();
    }
    
    std::cout << "[OmniverseLiveSync] Saved " << count << " changes" << std::endl;
    return count;
}

int OmniverseLiveSync::PushChanges(
    const std::string& session_id,
    const std::vector<SceneChange>& changes) {
    
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return 0;
    }
    
    auto& session = it->second;
    session.pending_changes.insert(
        session.pending_changes.end(),
        changes.begin(),
        changes.end());
    
    session.info.change_count = session.pending_changes.size();
    session.info.total_changes += changes.size();
    
    std::cout << "[OmniverseLiveSync] Pushed " << changes.size() 
              << " changes to session " << session_id << std::endl;
    
    return changes.size();
}

std::vector<SceneChange> OmniverseLiveSync::PullChanges(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return {};
    }
    
    auto& session = it->second;
    std::vector<SceneChange> changes = session.incoming_changes;
    
    // Clear queue
    while (!session.incoming_changes.empty()) {
        session.incoming_changes.pop();
    }
    
    std::cout << "[OmniverseLiveSync] Pulled " << changes.size() 
              << " changes from session " << session_id << std::endl;
    
    return changes;
}

// ============= Conflict Handling =============

bool OmniverseLiveSync::ResolveConflict(
    const std::string& session_id,
    const ConflictInfo& conflict,
    MergeStrategy strategy) {
    
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }
    
    auto& session = it->second;
    
    // Apply merge strategy
    SceneChange resolved = ApplyMergeStrategy(conflict, strategy);
    
    // Add resolved change to history
    session.change_history.push_back(resolved);
    
    // Remove from conflicts
    session.info.conflict_count = std::max(0, session.info.conflict_count - 1);
    
    std::cout << "[OmniverseLiveSync] Resolved conflict at " << conflict.prim_path 
              << " with strategy " << static_cast<int>(strategy) << std::endl;
    
    return true;
}

std::vector<ConflictInfo> OmniverseLiveSync::GetConflicts(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return {};
    }
    
    return it->second.conflicts;
}

// ============= Subscriptions =============

void OmniverseLiveSync::SubscribeToChanges(
    const std::string& session_id,
    std::function<void(const SceneChange&)> callback) {
    
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        it->second.change_callback = callback;
        std::cout << "[OmniverseLiveSync] Subscribed to changes for " << session_id << std::endl;
    }
}

void OmniverseLiveSync::UnsubscribeFromChanges(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        it->second.change_callback = nullptr;
        std::cout << "[OmniverseLiveSync] Unsubscribed from changes for " << session_id << std::endl;
    }
}

// ============= State Queries =============

SessionState OmniverseLiveSync::GetSessionState(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return SessionState::DISCONNECTED;
    }
    
    return it->second.info.state;
}

OmniverseSessionInfo OmniverseLiveSync::GetSessionInfo(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return OmniverseSessionInfo();
    }
    
    return it->second.info;
}

std::vector<std::string> OmniverseLiveSync::ListActiveSessions() {
    std::vector<std::string> result;
    for (const auto& pair : sessions_) {
        result.push_back(pair.first);
    }
    return result;
}

std::vector<std::string> OmniverseLiveSync::GetConnectedUsers(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return {};
    }
    
    return it->second.info.connected_users;
}

// ============= Configuration =============

void OmniverseLiveSync::SetMergeStrategy(MergeStrategy strategy) {
    default_merge_strategy_ = strategy;
    std::cout << "[OmniverseLiveSync] Set default merge strategy to " 
              << static_cast<int>(strategy) << std::endl;
}

void OmniverseLiveSync::EnableAutoSync(bool enabled) {
    for (auto& pair : sessions_) {
        pair.second.auto_sync = enabled;
    }
    std::cout << "[OmniverseLiveSync] Auto sync: " << (enabled ? "enabled" : "disabled") << std::endl;
}

void OmniverseLiveSync::SetSyncInterval(double seconds) {
    for (auto& pair : sessions_) {
        pair.second.sync_interval_s = seconds;
    }
    std::cout << "[OmniverseLiveSync] Set sync interval to " << seconds << " seconds" << std::endl;
}

bool OmniverseLiveSync::ForceSync(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }
    
    auto& session = it->second;
    session.info.state = SessionState::SYNCING;
    
    // Simulate sync
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    session.info.state = SessionState::CONNECTED;
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    session.info.last_sync = std::ctime(&time);
    
    std::cout << "[OmniverseLiveSync] Forced sync on session " << session_id << std::endl;
    
    return true;
}

// ============= Private Implementation =============

void OmniverseLiveSync::SyncThread(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) return;
    
    auto& session = it->second;
    
    while (session.auto_sync) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<int>(session.sync_interval_s * 1000)));
        
        ProcessIncomingChanges(session_id);
        DetectConflicts(session_id);
    }
}

void OmniverseLiveSync::ProcessIncomingChanges(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) return;
    
    auto& session = it->second;
    
    while (!session.incoming_changes.empty()) {
        SceneChange change = session.incoming_changes.front();
        session.incoming_changes.pop();
        
        if (session.change_callback) {
            session.change_callback(change);
        }
    }
}

void OmniverseLiveSync::DetectConflicts(const std::string& session_id) {
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) return;
    
    auto& session = it->second;
    
    // Simplified conflict detection
    // In production, would compare timestamps, user IDs, and property changes
    
    session.info.conflict_count = session.conflicts.size();
}

SceneChange OmniverseLiveSync::ApplyMergeStrategy(
    const ConflictInfo& conflict,
    MergeStrategy strategy) {
    
    SceneChange result;
    
    switch (strategy) {
        case MergeStrategy::LAST_WRITE_WINS:
            // Accept remote change
            result = conflict.remote_change;
            break;
            
        case MergeStrategy::KEEP_LOCAL:
            // Keep local change
            result = conflict.local_change;
            break;
            
        case MergeStrategy::MERGE_PROPERTIES:
            // Merge non-conflicting properties
            result = conflict.remote_change;
            result.old_value = conflict.local_change.old_value;
            break;
            
        case MergeStrategy::MANUAL:
            // Use local by default, should be resolved by UI
            result = conflict.local_change;
            break;
    }
    
    return result;
}

} // namespace SZM::Omniverse
