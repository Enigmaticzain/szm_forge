#pragma once

#include "OmniverseLiveSync.hpp"
#include <vector>

namespace SZM::Omniverse {

/**
 * @class ConflictResolver
 * @brief Detects and resolves conflicts between local and remote changes
 * 
 * Implements multiple merge strategies for collaborative editing
 */
class ConflictResolver {
public:
    /**
     * @brief Detect conflicts between change sets
     * 
     * Identifies when local and remote changes modify the same property
     * 
     * @param local Local changes
     * @param remote Remote changes
     * @return Vector of conflicting changes
     */
    std::vector<ConflictInfo> DetectConflicts(
        const std::vector<SceneChange>& local,
        const std::vector<SceneChange>& remote);
    
    /**
     * @brief Check if two changes conflict
     * 
     * Changes conflict if they modify the same property
     * 
     * @param a First change
     * @param b Second change
     * @return true if conflict exists
     */
    bool AreConflicting(const SceneChange& a, const SceneChange& b) const;
    
    /**
     * @brief Resolve conflict using merge strategy
     * 
     * Applies the specified strategy to determine which change to keep
     * 
     * @param local Local change
     * @param remote Remote change
     * @param strategy Merge strategy
     * @return Resolved change
     */
    SceneChange Resolve(
        const SceneChange& local,
        const SceneChange& remote,
        MergeStrategy strategy);
    
    /**
     * @brief Merge non-conflicting properties
     * 
     * Combines changes that don't overlap
     * 
     * @param local Local changes
     * @param remote Remote changes
     * @return Merged changes
     */
    std::vector<SceneChange> MergeProperties(
        const std::vector<SceneChange>& local,
        const std::vector<SceneChange>& remote);
    
    /**
     * @brief Apply three-way merge
     * 
     * Merges local and remote changes against a common ancestor
     * 
     * @param ancestor Ancestor version
     * @param local Local changes
     * @param remote Remote changes
     * @return Merged result
     */
    std::vector<SceneChange> ThreeWayMerge(
        const std::vector<SceneChange>& ancestor,
        const std::vector<SceneChange>& local,
        const std::vector<SceneChange>& remote);

private:
    /**
     * @brief Get property key for comparison
     */
    std::string GetPropertyKey(const SceneChange& change) const;
    
    /**
     * @brief Check if changes are on same property
     */
    bool IsSameProperty(const SceneChange& a, const SceneChange& b) const;
};

} // namespace SZM::Omniverse
