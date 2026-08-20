#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <utility>
#include <optional>

namespace SZM::Materials {

/**
 * @struct PlasticityCurve
 * @brief Represents non-linear material behavior post-yield.
 * Stores a piecewise linear true stress vs. plastic strain curve.
 */
struct PlasticityCurve {
    std::vector<std::pair<double, double>> dataPoints; // {plastic strain, true stress [MPa]}
};

/**
 * @struct IMaterial
 * @brief Complete isotropic linear-elastic material descriptor.
 *
 * Stores mechanical and thermal properties required by:
 *   - FEA solver (Module 320-Series)
 *   - Physics engine inertia calculations (Module 210-Series)
 *   - Thermal simulation (Module 300-Series — pending)
 *
 * The Shear Modulus G is auto-derived from E and ν on construction
 * to eliminate inconsistency errors in engineer-supplied data.
 *
 * Unit convention:
 *   E, G        → GPa
 *   Yield, UTS  → MPa
 *   Density     → kg/m³
 *   α           → 1/K
 *   k           → W/(m·K)
 */
struct IMaterial {
    std::string id;
    std::string name;

    // --- Mechanical ---------------------------------------------------
    double youngsModulus;       ///< E  — Elastic modulus [GPa]
    double poissonsRatio;       ///< ν  — Poisson's ratio [dimensionless]
    double shearModulus;        ///< G  — Auto-derived: G = E / 2(1+ν) [GPa]
    double yieldStrength;       ///< σ_y  — Yield stress [MPa]
    double ultimateStrength;    ///< σ_uts — Ultimate tensile stress [MPa]
    double density;             ///< ρ  — Mass density [kg/m³]

    // --- Thermal (optional — defaults to 0) --------------------------
    double thermalExpansion;    ///< α  — CTE [1/K]
    double thermalConductivity; ///< k  — Thermal conductivity [W/(m·K)]

    // --- Metadata (optional — from knowledge base) -------------------
    std::string category;           ///< e.g. "Ferrous / Alloy Steel"
    std::vector<std::string> tags;  ///< Searchable tags
    std::string notes;              ///< Free-form description
    std::string source;             ///< Provenance / reference

    // --- Advanced (optional) -----------------------------------------
    std::optional<PlasticityCurve> plasticity; ///< True stress vs plastic strain

    /**
     * @brief Primary constructor — validates ν and derives G automatically.
     */
    IMaterial(std::string_view matId,
              std::string_view matName,
              double e, double nu,
              double yield, double uts, double rho,
              double alpha = 0.0,
              double k     = 0.0)
        : id(matId), name(matName)
        , youngsModulus(e), poissonsRatio(nu)
        , yieldStrength(yield), ultimateStrength(uts)
        , density(rho)
        , thermalExpansion(alpha), thermalConductivity(k)
    {
        // Physical validity check: ν must be in (−1, 0.5) for isotropic materials
        if (nu <= -1.0 || nu >= 0.5)
            throw std::invalid_argument(
                "[SZM Materials] Poisson's ratio " + std::to_string(nu) +
                " is outside the physically valid range (−1, 0.5).");

        // G = E / 2(1+ν)
        shearModulus = e / (2.0 * (1.0 + nu));
    }

    /// Default constructor — required for map/container default-init
    IMaterial() = default;
};

} // namespace SZM::Materials
