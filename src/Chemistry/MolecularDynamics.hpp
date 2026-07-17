#pragma once

#include <vector>
#include <cmath>

namespace SZM {
namespace Chemistry {

/**
 * @brief Open-source inspired Molecular Dynamics (MD) Engine.
 * Implements fundamental models for computational chemistry such as the Lennard-Jones potential
 * and Velocity Verlet integration, commonly used in open-source MD simulators (e.g., LAMMPS, GROMACS).
 */

struct Particle {
    double x, y, z;       // Position
    double vx, vy, vz;    // Velocity
    double fx, fy, fz;    // Force
    double mass;          // Atomic mass
    int type;             // Particle type (e.g., 0 = Argon, 1 = Helium)
};

class MolecularDynamics {
public:
    MolecularDynamics(double epsilon, double sigma);

    void AddParticle(double x, double y, double z, double mass = 1.0);

    /**
     * @brief Computes forces between all pairs of particles using Lennard-Jones potential.
     * V(r) = 4 * epsilon * [(sigma/r)^12 - (sigma/r)^6]
     * F(r) = -dV/dr
     */
    void ComputeForces();

    /**
     * @brief Velocity Verlet Integrator.
     * Symplectic integrator, highly stable for conservative systems.
     */
    void StepVerlet(double dt);

    const std::vector<Particle>& GetParticles() const;

private:
    std::vector<Particle> m_Particles;
    double m_Epsilon; // Depth of potential well
    double m_Sigma;   // Distance at which potential is zero
    double m_Cutoff;  // Interaction cutoff radius
};

} // namespace Chemistry
} // namespace SZM
