#include "MaterialDatabase.hpp"
#include <algorithm>
#include <iostream>
#include <mutex>

namespace SZM::Materials {

// ------------------------------------------------- LoadStandardLibraries
void MaterialDatabase::LoadStandardLibraries() {
    std::unique_lock lock(m_Mutex); // Exclusive write lock

    // In production: parse 'materials.json' or query 'materials.db'.
    // Hardcoded here so the system is self-contained for v0.2.

    // Structural Steel — AISI 1020 approximation
    // E=200 GPa | ν=0.30 | σ_y=250 MPa | UTS=400 MPa | ρ=7850 kg/m³
    // α=1.2×10⁻⁵ /K | k=50 W/(m·K)
    IMaterial structuralSteel(
        "MAT-STEEL-STRUCT", "Structural Steel",
        200.0, 0.30, 250.0, 400.0, 7850.0,
        1.2e-5, 50.0
    );
    structuralSteel.plasticity = PlasticityCurve{{
        {0.0, 250.0},
        {0.02, 300.0},
        {0.1, 400.0},
        {0.2, 450.0}
    }};

    // 6061-T6 Aluminium
    // E=68.9 GPa | ν=0.33 | σ_y=276 MPa | UTS=310 MPa | ρ=2700 kg/m³
    // α=2.32×10⁻⁵ /K | k=167 W/(m·K)
    IMaterial al6061(
        "MAT-AL-6061-T6", "6061-T6 Aluminum",
        68.9, 0.33, 276.0, 310.0, 2700.0,
        2.32e-5, 167.0
    );
    al6061.plasticity = PlasticityCurve{{
        {0.0, 276.0},
        {0.05, 300.0},
        {0.12, 310.0}
    }};

    // Titanium Ti-6Al-4V (Grade 5) — common in aerospace
    IMaterial ti6al4v(
        "MAT-TI-6AL4V", "Ti-6Al-4V Titanium",
        113.8, 0.342, 880.0, 950.0, 4430.0,
        8.6e-6, 6.7
    );

    // C110 copper — common high-conductivity engineering copper
    IMaterial c110Copper(
        "MAT-CU-C110", "C110 Copper",
        117.0, 0.34, 69.0, 220.0, 8960.0,
        1.68e-5, 385.0
    );

    m_Registry[structuralSteel.id] = structuralSteel;
    m_Registry[al6061.id]          = al6061;
    m_Registry[ti6al4v.id]         = ti6al4v;
    m_Registry[c110Copper.id]      = c110Copper;

    std::cout << "[SZM Materials] Standard library loaded ("
              << m_Registry.size() << " materials).\n";
}

// -------------------------------------------------- AddCustomMaterial ---
bool MaterialDatabase::AddCustomMaterial(const IMaterial& material) {
    std::unique_lock lock(m_Mutex);
    auto [it, inserted] = m_Registry.insert({ material.id, material });
    return inserted;
}

// ------------------------------------------------------- GetMaterial ----
std::optional<IMaterial> MaterialDatabase::GetMaterial(const std::string& id) const {
    std::shared_lock lock(m_Mutex); // Shared read — multiple threads OK
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
