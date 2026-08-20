#include "LiveSyncSession.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>

namespace SZM::Omniverse {

// ============= Constructor/Destructor =============

LiveSyncSession::LiveSyncSession(const std::string& session_id)
    : session_id_(session_id),
      latency_ms_(50.0),
      change_count_(0),
      state_(SessionState::CONNECTED),
      is_active_(false) {
    std::cout << "[LiveSyncSession] Created session: " << session_id << std::endl;
}

// ============= Session Lifecycle =============

void LiveSyncSession::Start() {
    std::lock_guard<std::mutex> lock(mutex_);
    is_active_ = true;
    state_ = SessionState::CONNECTED;
    std::cout << "[LiveSyncSession] Started session: " << session_id_ << std::endl;
}

void LiveSyncSession::Stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    is_active_ = false;
    state_ = SessionState::DISCONNECTED;
    std::cout << "[LiveSyncSession] Stopped session: " << session_id_ << std::endl;
}

bool LiveSyncSession::IsActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return is_active_;
}

// ============= User Management =============

void LiveSyncSession::AddUser(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(connected_users_.begin(), connected_users_.end(), user_id);
    if (it == connected_users_.end()) {
        connected_users_.push_back(user_id);
        std::cout << "[LiveSyncSession] Added user: " << user_id 
                  << " (total: " << connected_users_.size() << ")" << std::endl;
    }
}

void LiveSyncSession::RemoveUser(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(connected_users_.begin(), connected_users_.end(), user_id);
    if (it != connected_users_.end()) {
        connected_users_.erase(it);
        std::cout << "[LiveSyncSession] Removed user: " << user_id 
                  << " (remaining: " << connected_users_.size() << ")" << std::endl;
    }
}

std::vector<std::string> LiveSyncSession::GetConnectedUsers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connected_users_;
}

// ============= Change Tracking =============

void LiveSyncSession::RecordChange(const SceneChange& change) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    pending_changes_.push_back(change);
    change_history_.push_back(change);
    change_count_++;
    
    std::cout << "[LiveSyncSession] Recorded change: " << change.prim_path 
              << " (total: " << change_count_ << ")" << std::endl;
}

std::vector<SceneChange> LiveSyncSession::GetPendingChanges() {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_changes_;
}

void LiveSyncSession::ClearPendingChanges() {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_changes_.clear();
    std::cout << "[LiveSyncSession] Cleared pending changes" << std::endl;
}

std::vector<SceneChange> LiveSyncSession::GetChangeHistory() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return change_history_;
}

// ============= State Synchronization =============

void LiveSyncSession::SyncState() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_active_) return;
    
    state_ = SessionState::SYNCING;
    
    // Simulate sync delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    state_ = SessionState::CONNECTED;
    
    std::cout << "[LiveSyncSession] Sync completed for session: " << session_id_ << std::endl;
}

void LiveSyncSession::ApplyRemoteChanges(const std::vector<SceneChange>& changes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& change : changes) {
        change_history_.push_back(change);
    }
    
    std::cout << "[LiveSyncSession] Applied " << changes.size() 
              << " remote changes to session: " << session_id_ << std::endl;
}

// ============= Conflict Detection =============

bool LiveSyncSession::HasConflicts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !conflicts_.empty();
}

std::vector<ConflictInfo> LiveSyncSession::GetConflicts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return conflicts_;
}

void LiveSyncSession::ResolveConflict(const SceneChange& change, MergeStrategy strategy) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Remove conflict from list
    // (In production would find specific conflict)
    if (!conflicts_.empty()) {
        conflicts_.pop_back();
    }
    
    // Add resolved change to history
    change_history_.push_back(change);
    
    std::cout << "[LiveSyncSession] Resolved conflict with strategy: " 
              << static_cast<int>(strategy) << std::endl;
}

// ============= Getters =============

std::string LiveSyncSession::GetSessionId() const {
    return session_id_;
}

double LiveSyncSession::GetLatency() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latency_ms_;
}

int LiveSyncSession::GetChangeCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return change_count_;
}

SessionState LiveSyncSession::GetState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

} // namespace SZM::Omniverse
