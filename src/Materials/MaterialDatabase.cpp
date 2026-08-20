#include "MaterialDatabase.hpp"
#include "../Knowledge/KnowledgePathUtils.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>

namespace SZM::Materials {

namespace {

void ApplyOptionalMetadata(IMaterial& m, const nlohmann::json& j) {
    if (j.contains("category") && j["category"].is_string()) {
        m.category = j["category"].get<std::string>();
    }
    if (j.contains("notes") && j["notes"].is_string()) {
        m.notes = j["notes"].get<std::string>();
    }
    if (j.contains("source") && j["source"].is_string()) {
        m.source = j["source"].get<std::string>();
    }
    if (j.contains("tags") && j["tags"].is_array()) {
        for (const auto& t : j["tags"]) {
            if (t.is_string()) {
                m.tags.push_back(t.get<std::string>());
            }
        }
    }
}

std::optional<IMaterial> ParseMaterialJson(const nlohmann::json& j) {
    if (!j.contains("id") || !j.contains("name")) {
        return std::nullopt;
    }

    const double e     = j.value("youngsModulus_GPa", 0.0);
    const double nu    = j.value("poissonsRatio", 0.0);
    const double yield = j.value("yieldStrength_MPa", 0.0);
    const double uts   = j.value("ultimateStrength_MPa", yield);
    const double rho   = j.value("density_kg_m3", 0.0);
    const double alpha = j.value("thermalExpansion_1_K", 0.0);
    const double k     = j.value("thermalConductivity_W_mK", 0.0);

    if (e <= 0.0 || rho <= 0.0) {
        return std::nullopt;
    }

    try {
        IMaterial m(j["id"].get<std::string>(),
                    j["name"].get<std::string>(),
                    e, nu, yield, uts, rho, alpha, k);
        ApplyOptionalMetadata(m, j);
        return m;
    } catch (const std::exception& ex) {
        std::cerr << "[SZM Materials] Skip invalid material " << j.value("id", "?")
                  << ": " << ex.what() << "\n";
        return std::nullopt;
    }
}

} // namespace

// ------------------------------------------------- LoadBuiltinMaterials
void MaterialDatabase::LoadBuiltinMaterials() {
    std::unique_lock lock(m_Mutex);
    m_Registry.clear();

    IMaterial structuralSteel(
        "MAT-STEEL-STRUCT", "Structural Steel",
        200.0, 0.30, 250.0, 400.0, 7850.0,
        1.2e-5, 50.0
    );
    structuralSteel.category = "Ferrous / Low Carbon Steel";
    structuralSteel.tags     = {"steel", "AISI 1020", "structural"};
    structuralSteel.plasticity = PlasticityCurve{{
        {0.0, 250.0},
        {0.02, 300.0},
        {0.1, 400.0},
        {0.2, 450.0}
    }};

    IMaterial al6061(
        "MAT-AL-6061-T6", "6061-T6 Aluminum",
        68.9, 0.33, 276.0, 310.0, 2700.0,
        2.32e-5, 167.0
    );
    al6061.category = "Non-Ferrous / Aluminum";
    al6061.tags     = {"aluminum", "6061"};
    al6061.plasticity = PlasticityCurve{{
        {0.0, 276.0},
        {0.05, 300.0},
        {0.12, 310.0}
    }};

    IMaterial ti6al4v(
        "MAT-TI-6AL4V", "Ti-6Al-4V Titanium",
        113.8, 0.342, 880.0, 950.0, 4430.0,
        8.6e-6, 6.7
    );
    ti6al4v.category = "Non-Ferrous / Titanium";
    ti6al4v.tags     = {"titanium", "aerospace"};

    IMaterial c110Copper(
        "MAT-CU-C110", "C110 Copper",
        117.0, 0.34, 69.0, 220.0, 8960.0,
        1.68e-5, 385.0
    );
    c110Copper.category = "Non-Ferrous / Copper";
    c110Copper.tags     = {"copper", "electrical"};

    m_Registry[structuralSteel.id] = structuralSteel;
    m_Registry[al6061.id]          = al6061;
    m_Registry[ti6al4v.id]         = ti6al4v;
    m_Registry[c110Copper.id]      = c110Copper;

    std::cout << "[SZM Materials] Built-in library loaded ("
              << m_Registry.size() << " materials).\n";
}

// ------------------------------------------------- LoadFromFile ----------
std::size_t MaterialDatabase::LoadFromFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) {
        return 0;
    }

    nlohmann::json root;
    try {
        in >> root;
    } catch (const std::exception& ex) {
        std::cerr << "[SZM Materials] Failed to parse " << path << ": " << ex.what() << "\n";
        return 0;
    }

    if (!root.contains("entries") || !root["entries"].is_array()) {
        return 0;
    }

    std::unique_lock lock(m_Mutex);
    m_Registry.clear();

    std::size_t loaded = 0;
    for (const auto& entry : root["entries"]) {
        auto mat = ParseMaterialJson(entry);
        if (!mat.has_value()) {
            continue;
        }
        m_Registry[mat->id] = *mat;
        ++loaded;
    }

    if (loaded > 0) {
        std::cout << "[SZM Materials] Loaded " << loaded << " materials from "
                  << path.string() << ".\n";
    }
    return loaded;
}

// ------------------------------------------------- LoadStandardLibraries
void MaterialDatabase::LoadStandardLibraries() {
    const auto kbDir = Knowledge::ResolveKnowledgeDirectory();
    if (!kbDir.empty()) {
        const auto matFile = kbDir / "materials.json";
        if (std::filesystem::exists(matFile)) {
            if (LoadFromFile(matFile) > 0) {
                return;
            }
        }
    }

    LoadBuiltinMaterials();
}

// -------------------------------------------------- AddCustomMaterial ---
bool MaterialDatabase::AddCustomMaterial(const IMaterial& material) {
    std::unique_lock lock(m_Mutex);
    auto [it, inserted] = m_Registry.insert({ material.id, material });
    return inserted;
}

// ------------------------------------------------------- GetMaterial ----
std::optional<IMaterial> MaterialDatabase::GetMaterial(const std::string& id) const {
    std::shared_lock lock(m_Mutex);
    auto it = m_Registry.find(id);
    if (it != m_Registry.end())
        return it->second;
    return std::nullopt;
}

std::vector<IMaterial> MaterialDatabase::GetAllMaterials() const {
    std::shared_lock lock(m_Mutex);

    std::vector<IMaterial> materials;
    materials.reserve(m_Registry.size());

    for (const auto& [id, material] : m_Registry) {
        (void)id;
        materials.push_back(material);
    }

    std::sort(materials.begin(), materials.end(),
              [](const IMaterial& lhs, const IMaterial& rhs) {
                  if (lhs.name == rhs.name) {
                      return lhs.id < rhs.id;
                  }
                  return lhs.name < rhs.name;
              });

    return materials;
}

} // namespace SZM::Materials
