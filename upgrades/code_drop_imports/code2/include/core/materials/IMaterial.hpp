#pragma once
#include <string>
#include <stdexcept>

namespace szm {
namespace materials {

struct IMaterial {
    std::string id;
    std::string name;

    // Mechanical Properties
    double youngsModulus;     // E in GPa
    double poissonsRatio;     // ν (0.0 to 0.5)
    double shearModulus;      // G in GPa (Derived internally)
    double yieldStrength;     // σ_y in MPa
    double ultimateStrength;  // σ_uts in MPa
    double density;           // ρ in kg/m^3 (Required for mass/dynamics)

    // Thermal Properties
    double thermalExpansion;  // α in 1/K
    double thermalConductivity; // k in W/(m·K)

    // Constructor automates the derivation of the Shear Modulus (G)
    IMaterial(std::string_view matId, std::string_view matName, 
              double e, double nu, double yield, double uts, double rho, 
              double alpha = 0.0, double k = 0.0)
        : id(matId), name(matName), youngsModulus(e), poissonsRatio(nu), 
          yieldStrength(yield), ultimateStrength(uts), density(rho), 
          thermalExpansion(alpha), thermalConductivity(k) 
    {
        if (nu <= -1.0 || nu >= 0.5) {
            throw std::invalid_argument("Poisson's ratio must be physically valid for isotropic materials.");
        }
        // Derive Shear Modulus: G = E / 2(1+ν)
        shearModulus = youngsModulus / (2.0 * (1.0 + poissonsRatio));
    }

    // Default constructor for map initialization
    IMaterial() = default;
};

} // namespace materials
} // namespace szm
