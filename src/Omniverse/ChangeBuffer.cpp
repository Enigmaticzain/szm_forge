#include "ChangeBuffer.hpp"
#include <iostream>

namespace SZM::Omniverse {

void ChangeBuffer::AddChange(const SceneChange& change) {
    buffer_.push_back(change);
    
    // Track latest change for this property
    std::string key = change.prim_path + ":" + change.property_name;
    latest_[key] = change;
}

std::vector<SceneChange> ChangeBuffer::Compress() {
    std::vector<SceneChange> compressed;
    
    original_size_ = buffer_.size();
    
    // Use only the latest change for each property
    for (const auto& pair : latest_) {
        compressed.push_back(pair.second);
    }
    
    std::cout << "[ChangeBuffer] Compressed " << original_size_ 
              << " changes to " << compressed.size() 
              << " (ratio: " << GetCompressionRatio() << "x)" << std::endl;
    
    return compressed;
}

std::vector<SceneChange> ChangeBuffer::GetDeltas() const {
    // Return all changes currently in buffer
    return buffer_;
}

void ChangeBuffer::Clear() {
    buffer_.clear();
    latest_.clear();
    original_size_ = 0;
    std::cout << "[ChangeBuffer] Cleared buffer" << std::endl;
}

size_t ChangeBuffer::Size() const {
    return buffer_.size();
}

bool ChangeBuffer::IsEmpty() const {
    return buffer_.empty();
}

double ChangeBuffer::GetCompressionRatio() const {
    if (original_size_ == 0) return 1.0;
    return static_cast<double>(original_size_) / latest_.size();
}

} // namespace SZM::Omniverse
