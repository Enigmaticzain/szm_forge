#pragma once

#include "IMaterial.hpp"
#include <filesystem>
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <string>
#include <vector>

namespace SZM::Materials {

/**
 * @class MaterialDatabase
 * @brief Thread-safe singleton registry of all physical materials.
 *
 * Uses std::shared_mutex so multiple FEA solver threads can read
 * concurrently while the loader holds an exclusive write lock.
 *
 * Standard library IDs (loaded by loadStandardLibraries()):
 *   "MAT-STEEL-STRUCT"  — Structural Steel (AISI 1020 approx)
 *   "MAT-AL-6061-T6"    — 6061-T6 Aluminium
 *
 * Usage:
 *   auto mat = MaterialDatabase::GetInstance().GetMaterial("MAT-STEEL-STRUCT");
 */
class MaterialDatabase {
public:
    static MaterialDatabase& GetInstance() {
        static MaterialDatabase instance;
        return instance;
    }

    MaterialDatabase(const MaterialDatabase&)            = delete;
    MaterialDatabase& operator=(const MaterialDatabase&) = delete;

    /// Load the built-in engineering material library.
    /// Tries data/knowledge/materials.json first; falls back to hardcoded defaults.
    void LoadStandardLibraries();

    /// Load materials from a JSON knowledge-base file.
    /// Returns number of materials loaded.
    [[nodiscard]] std::size_t LoadFromFile(const std::filesystem::path& path);

    /// Register hardcoded fallback materials (4 baseline grades).
    void LoadBuiltinMaterials();

    /// Register a custom material. Returns false if the ID is already taken.
    bool AddCustomMaterial(const IMaterial& material);

    /// Thread-safe lookup by material ID.
    /// Returns std::nullopt if the ID is not found.
    [[nodiscard]] std::optional<IMaterial> GetMaterial(const std::string& id) const;

    /// Thread-safe snapshot of all registered materials.
    /// Results are returned by value and sorted by material name for stable UI use.
    [[nodiscard]] std::vector<IMaterial> GetAllMaterials() const;

private:
    MaterialDatabase()  = default;
    ~MaterialDatabase() = default;

    std::unordered_map<std::string, IMaterial> m_Registry;
    mutable std::shared_mutex                  m_Mutex;
};

} // namespace SZM::Materials
