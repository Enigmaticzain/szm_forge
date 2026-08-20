#include "ConflictResolver.hpp"
#include <iostream>
#include <algorithm>

namespace SZM::Omniverse {

// ============= Conflict Detection =============

std::vector<ConflictInfo> ConflictResolver::DetectConflicts(
    const std::vector<SceneChange>& local,
    const std::vector<SceneChange>& remote) {
    
    std::vector<ConflictInfo> conflicts;
    
    for (const auto& local_change : local) {
        for (const auto& remote_change : remote) {
            if (AreConflicting(local_change, remote_change)) {
                ConflictInfo conflict;
                conflict.conflict_id = local_change.prim_path + "_" + 
                                      std::to_string(static_cast<int>(local_change.type));
                conflict.local_change = local_change;
                conflict.remote_change = remote_change;
                conflict.prim_path = local_change.prim_path;
                conflict.timestamp = local_change.timestamp;
                
                conflicts.push_back(conflict);
                
                std::cout << "[ConflictResolver] Detected conflict at: " 
                         << conflict.prim_path << std::endl;
            }
        }
    }
    
    return conflicts;
}

bool ConflictResolver::AreConflicting(const SceneChange& a, const SceneChange& b) const {
    // Two changes conflict if they modify the same property
    return IsSameProperty(a, b) && a.type == b.type;
}

// ============= Conflict Resolution =============

SceneChange ConflictResolver::Resolve(
    const SceneChange& local,
    const SceneChange& remote,
    MergeStrategy strategy) {
    
    SceneChange result;
    
    switch (strategy) {
        case MergeStrategy::LAST_WRITE_WINS:
            // Use remote (assumed to be more recent)
            result = remote;
            std::cout << "[ConflictResolver] Using remote change (LAST_WRITE_WINS)" << std::endl;
            break;
            
        case MergeStrategy::KEEP_LOCAL:
            // Keep local change
            result = local;
            std::cout << "[ConflictResolver] Keeping local change (KEEP_LOCAL)" << std::endl;
            break;
            
        case MergeStrategy::MERGE_PROPERTIES:
            // Try to merge non-overlapping properties
            result = remote;
            result.old_value = local.old_value;  // Keep local old value
            std::cout << "[ConflictResolver] Merged properties" << std::endl;
            break;
            
        case MergeStrategy::MANUAL:
            // Default to local pending manual resolution
            result = local;
            std::cout << "[ConflictResolver] Pending manual resolution" << std::endl;
            break;
    }
    
    return result;
}

// ============= Property Merging =============

std::vector<SceneChange> ConflictResolver::MergeProperties(
    const std::vector<SceneChange>& local,
    const std::vector<SceneChange>& remote) {
    
    std::vector<SceneChange> merged;
    
    // Add all local changes
    for (const auto& change : local) {
        merged.push_back(change);
    }
    
    // Add remote changes that don't conflict
    for (const auto& remote_change : remote) {
        bool conflicts = false;
        for (const auto& local_change : local) {
            if (AreConflicting(local_change, remote_change)) {
                conflicts = true;
                break;
            }
        }
        
        if (!conflicts) {
            merged.push_back(remote_change);
        }
    }
    
    std::cout << "[ConflictResolver] Merged " << local.size() 
              << " local and " << remote.size() 
              << " remote changes into " << merged.size() << std::endl;
    
    return merged;
}

// ============= Three-Way Merge =============

std::vector<SceneChange> ConflictResolver::ThreeWayMerge(
    const std::vector<SceneChange>& ancestor,
    const std::vector<SceneChange>& local,
    const std::vector<SceneChange>& remote) {
    
    std::vector<SceneChange> merged;
    
    // Find what changed from ancestor to local
    std::vector<SceneChange> local_edits;
    for (const auto& local_change : local) {
        bool in_ancestor = false;
        for (const auto& ancestor_change : ancestor) {
            if (IsSameProperty(local_change, ancestor_change)) {
                in_ancestor = true;
                break;
            }
        }
        if (!in_ancestor) {
            local_edits.push_back(local_change);
        }
    }
    
    // Find what changed from ancestor to remote
    std::vector<SceneChange> remote_edits;
    for (const auto& remote_change : remote) {
        bool in_ancestor = false;
        for (const auto& ancestor_change : ancestor) {
            if (IsSameProperty(remote_change, ancestor_change)) {
                in_ancestor = true;
                break;
            }
        }
        if (!in_ancestor) {
            remote_edits.push_back(remote_change);
        }
    }
    
    // Start with ancestor
    merged = ancestor;
    
    // Apply non-conflicting edits
    for (const auto& edit : local_edits) {
        bool conflicts = false;
        for (const auto& remote_edit : remote_edits) {
            if (IsSameProperty(edit, remote_edit)) {
                conflicts = true;
                break;
            }
        }
        if (!conflicts) {
            merged.push_back(edit);
        }
    }
    
    for (const auto& edit : remote_edits) {
        bool conflicts = false;
        for (const auto& local_edit : local_edits) {
            if (IsSameProperty(edit, local_edit)) {
                conflicts = true;
                break;
            }
        }
        if (!conflicts) {
            merged.push_back(edit);
        }
    }
    
    std::cout << "[ConflictResolver] Three-way merge: ancestor=" << ancestor.size()
              << " local=" << local.size() << " remote=" << remote.size()
              << " merged=" << merged.size() << std::endl;
    
    return merged;
}

// ============= Private Helpers =============

std::string ConflictResolver::GetPropertyKey(const SceneChange& change) const {
    return change.prim_path + ":" + change.property_name;
}

bool ConflictResolver::IsSameProperty(const SceneChange& a, const SceneChange& b) const {
    return a.prim_path == b.prim_path && a.property_name == b.property_name;
}

} // namespace SZM::Omniverse
