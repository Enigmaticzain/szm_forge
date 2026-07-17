#include "MNASolver.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace SZM {
namespace Electrical {

MNASolver::MNASolver() : m_NumNodes(0), m_NumVoltageSources(0) {}

void MNASolver::AddComponent(const std::string& name, Component::Type type, int n1, int n2, double value) {
    m_Components.push_back({name, type, n1, n2, value});
}

void MNASolver::Clear() {
    m_Components.clear();
    m_NumNodes = 0;
    m_NumVoltageSources = 0;
}

void MNASolver::DetermineSystemSize() {
    m_NumNodes = 0;
    m_NumVoltageSources = 0;
    
    for (const auto& comp : m_Components) {
        if (comp.node1 > m_NumNodes) m_NumNodes = comp.node1;
        if (comp.node2 > m_NumNodes) m_NumNodes = comp.node2;
        if (comp.type == Component::Type::VoltageSource) {
            m_NumVoltageSources++;
        }
    }
}

void MNASolver::StampMatrix() {
    int dim = m_NumNodes + m_NumVoltageSources;
    m_A = Eigen::MatrixXd::Zero(dim, dim);
    m_z = Eigen::VectorXd::Zero(dim);

    int vSrcIndex = m_NumNodes; // Voltage source unknown currents start after node voltages

    for (const auto& comp : m_Components) {
        int n1 = comp.node1 - 1; // 0-indexed for matrix (Node 0 is ground, ignored in matrix)
        int n2 = comp.node2 - 1;

        if (comp.type == Component::Type::Resistor) {
            double g = 1.0 / comp.value; // Conductance
            // Stamp diagonal
            if (n1 >= 0) m_A(n1, n1) += g;
            if (n2 >= 0) m_A(n2, n2) += g;
            // Stamp off-diagonal
            if (n1 >= 0 && n2 >= 0) {
                m_A(n1, n2) -= g;
                m_A(n2, n1) -= g;
            }
        } 
        else if (comp.type == Component::Type::CurrentSource) {
            // Current enters node 1, leaves node 2
            if (n1 >= 0) m_z(n1) -= comp.value;
            if (n2 >= 0) m_z(n2) += comp.value;
        } 
        else if (comp.type == Component::Type::VoltageSource) {
            // Stamp B and C matrices (off-diagonal +1, -1 for vsrc)
            if (n1 >= 0) {
                m_A(n1, vSrcIndex) += 1.0;
                m_A(vSrcIndex, n1) += 1.0;
            }
            if (n2 >= 0) {
                m_A(n2, vSrcIndex) -= 1.0;
                m_A(vSrcIndex, n2) -= 1.0;
            }
            // Stamp z vector
            m_z(vSrcIndex) = comp.value;
            vSrcIndex++;
        }
    }
}

bool MNASolver::Solve() {
    if (m_Components.empty()) return false;
    
    DetermineSystemSize();
    StampMatrix();

    // Use LU decomposition for solving A * x = z
    // Eigen's PartialPivLU is fast and robust for general invertible matrices
    Eigen::PartialPivLU<Eigen::MatrixXd> lu(m_A);
    
    if (lu.matrixLU().diagonal().cwiseAbs().minCoeff() < 1e-12) {
        std::cerr << "[MNA] System matrix is singular! Check circuit for floating nodes or shorted voltage sources.\n";
        return false;
    }

    m_x = lu.solve(m_z);
    return true;
}

double MNASolver::GetNodeVoltage(int node) const {
    if (node == 0) return 0.0; // Ground is always 0V
    if (node > 0 && node <= m_NumNodes) {
        return m_x(node - 1);
    }
    return 0.0;
}

double MNASolver::GetVoltageSourceCurrent(const std::string& vsrcName) const {
    int vSrcIndex = m_NumNodes;
    for (const auto& comp : m_Components) {
        if (comp.type == Component::Type::VoltageSource) {
            if (comp.name == vsrcName) {
                return m_x(vSrcIndex); // Returns current through source (from node 1 to node 2)
            }
            vSrcIndex++;
        }
    }
    return 0.0;
}

} // namespace Electrical
} // namespace SZM
