#include "MolecularDynamics.hpp"

namespace SZM {
namespace Chemistry {

MolecularDynamics::MolecularDynamics(double epsilon, double sigma) 
    : m_Epsilon(epsilon), m_Sigma(sigma), m_Cutoff(2.5 * sigma) {}

void MolecularDynamics::AddParticle(double x, double y, double z, double mass) {
    m_Particles.push_back({x, y, z, 0, 0, 0, 0, 0, 0, mass, 0});
}

void MolecularDynamics::ComputeForces() {
    for (auto& p : m_Particles) {
        p.fx = p.fy = p.fz = 0.0;
    }

    double sig6 = std::pow(m_Sigma, 6);
    double sig12 = sig6 * sig6;

    for (size_t i = 0; i < m_Particles.size(); ++i) {
        for (size_t j = i + 1; j < m_Particles.size(); ++j) {
            double dx = m_Particles[i].x - m_Particles[j].x;
            double dy = m_Particles[i].y - m_Particles[j].y;
            double dz = m_Particles[i].z - m_Particles[j].z;
            
            double r2 = dx*dx + dy*dy + dz*dz;
            
            if (r2 < (m_Cutoff * m_Cutoff) && r2 > 0.0001) { // Prevent division by zero
                double r2inv = 1.0 / r2;
                double r6inv = r2inv * r2inv * r2inv;
                
                double fmag_div_r = 24.0 * m_Epsilon * r2inv * r6inv * (2.0 * sig12 * r6inv - sig6);
                
                m_Particles[i].fx += fmag_div_r * dx;
                m_Particles[i].fy += fmag_div_r * dy;
                m_Particles[i].fz += fmag_div_r * dz;
                
                m_Particles[j].fx -= fmag_div_r * dx;
                m_Particles[j].fy -= fmag_div_r * dy;
                m_Particles[j].fz -= fmag_div_r * dz;
            }
        }
    }
}

void MolecularDynamics::StepVerlet(double dt) {
    for (auto& p : m_Particles) {
        double ax = p.fx / p.mass;
        double ay = p.fy / p.mass;
        double az = p.fz / p.mass;

        p.x += p.vx * dt + 0.5 * ax * dt * dt;
        p.y += p.vy * dt + 0.5 * ay * dt * dt;
        p.z += p.vz * dt + 0.5 * az * dt * dt;

        p.vx += 0.5 * ax * dt;
        p.vy += 0.5 * ay * dt;
        p.vz += 0.5 * az * dt;
    }

    ComputeForces();

    for (auto& p : m_Particles) {
        double ax = p.fx / p.mass;
        double ay = p.fy / p.mass;
        double az = p.fz / p.mass;

        p.vx += 0.5 * ax * dt;
        p.vy += 0.5 * ay * dt;
        p.vz += 0.5 * az * dt;
    }
}

const std::vector<Particle>& MolecularDynamics::GetParticles() const { 
    return m_Particles; 
}

} // namespace Chemistry
} // namespace SZM
