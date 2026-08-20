#pragma once

#include "OmniverseLiveSync.hpp"
#include <vector>
#include <map>
#include <string>

namespace SZM::Omniverse {

/**
 * @class ChangeBuffer
 * @brief Buffers and compresses scene changes for efficient transmission
 * 
 * Merges consecutive changes on the same property to reduce bandwidth
 */
class ChangeBuffer {
public:
    /**
     * @brief Add change to buffer
     * @param change Change to buffer
     */
    void AddChange(const SceneChange& change);
    
    /**
     * @brief Compress buffered changes
     * 
     * Combines multiple changes on the same property,
     * keeping only the most recent value
     * 
     * @return Compressed changes
     */
    std::vector<SceneChange> Compress();
    
    /**
     * @brief Get delta changes (only what changed)
     * 
     * Returns only properties that differ from last known state
     * 
     * @return Delta changes
     */
    std::vector<SceneChange> GetDeltas() const;
    
    /**
     * @brief Clear buffer
     */
    void Clear();
    
    /**
     * @brief Get buffer size
     * @return Number of buffered changes
     */
    size_t Size() const;
    
    /**
     * @brief Check if buffer is empty
     * @return true if empty
     */
    bool IsEmpty() const;
    
    /**
     * @brief Get compression ratio
     * @return Ratio of original to compressed size
     */
    double GetCompressionRatio() const;

private:
    std::vector<SceneChange> buffer_;
    std::map<std::string, SceneChange> latest_;  ///< Latest change per property
    size_t original_size_;
};

} // namespace SZM::Omniverse
