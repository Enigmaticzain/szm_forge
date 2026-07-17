#pragma once
#include "IMaterial.hpp"
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <string>

namespace szm {
namespace materials {

class MaterialDatabase {
public:
    // Globally accessible singleton instance
    static MaterialDatabase& getInstance() {
        static MaterialDatabase instance;
        return instance;
    }

    // Delete copy and move constructors
    MaterialDatabase(const MaterialDatabase&) = delete;
    MaterialDatabase& operator=(const MaterialDatabase&) = delete;

    // Core functionality
    void loadStandardLibraries();
    bool addCustomMaterial(const IMaterial& material);
    
    // Thread-safe, high-speed lookup
    std::optional<IMaterial> getMaterial(const std::string& id) const;

private:
    MaterialDatabase() = default;
    ~MaterialDatabase() = default;

    // Internal State
    std::unordered_map<std::string, IMaterial> m_materialRegistry;
    
    // shared_mutex allows multiple readers (solvers) or one writer (loader)
    mutable std::shared_mutex m_mutex; 
};

} // namespace materials
} // namespace szm

