#include "LinearSolver.hpp"
#include "../Materials/MaterialDatabase.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace SZM::FEA {

LinearStaticSolver::LinearStaticSolver(FEAMesh& mesh) : m_Mesh(mesh) {
    m_DOFCount = m_Mesh.Nodes.size() * 3;
    m_K.resize(static_cast<int>(m_DOFCount), static_cast<int>(m_DOFCount));
    m_F = Eigen::VectorXd::Zero(static_cast<int>(m_DOFCount));
    m_U = Eigen::VectorXd::Zero(static_cast<int>(m_DOFCount));
}

// ----------------------------------------------- AssembleSystem -----
void LinearStaticSolver::AssembleSystem() {
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(m_Mesh.Elements.size() * 144); // 12×12 per Tet

    // Assemble nodal force vector
    for (const auto& load : m_Mesh.Loads) {
        int dof = static_cast<int>(load.NodeID * 3);
        m_F(dof)   += load.Force.x();
        m_F(dof+1) += load.Force.y();
        m_F(dof+2) += load.Force.z();
    }

    // Assemble global stiffness [K] via element scatter
    for (const auto& el : m_Mesh.Elements) {
        Eigen::MatrixXd ke = ComputeElementStiffness(el);
        for (size_t i = 0; i < el.NodeIDs.size(); ++i) {
            for (size_t j = 0; j < el.NodeIDs.size(); ++j) {
                int row = static_cast<int>(el.NodeIDs[i]) * 3;
                int col = static_cast<int>(el.NodeIDs[j]) * 3;
                for (int d1 = 0; d1 < 3; ++d1)
                    for (int d2 = 0; d2 < 3; ++d2)
                        triplets.emplace_back(row+d1, col+d2,
                                              ke(static_cast<int>(i)*3+d1,
                                                 static_cast<int>(j)*3+d2));
            }
        }
    }

    m_K.setFromTriplets(triplets.begin(), triplets.end());
}

// -------------------------------------- ApplyBoundaryConditions -----
void LinearStaticSolver::ApplyBoundaryConditions() {
    for (const auto& bc : m_Mesh.BCs) {
        int base = static_cast<int>(bc.NodeID * 3);
        for (int i = 0; i < 3; ++i) {
            bool fixed = (i==0 && bc.FixX) || (i==1 && bc.FixY) || (i==2 && bc.FixZ);
            if (!fixed) continue;
            int dof = base + i;
            // Zero constrained row + column, set diagonal = 1.0 for a stable SPD system.
            for (int j = 0; j < static_cast<int>(m_DOFCount); ++j) {
                if (j == dof) continue;
                m_K.coeffRef(dof, j) = 0.0;
                m_K.coeffRef(j, dof) = 0.0;
            }
            m_K.coeffRef(dof, dof) = 1.0;
            m_F(dof) = 0.0;
        }
    }
    m_K.prune(0.0);
}

// ---------------------------------------------------------------- Solve
bool LinearStaticSolver::Solve() {
    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>,
                              Eigen::Lower | Eigen::Upper> cg;
    cg.compute(m_K);
    if (cg.info() != Eigen::Success) {
        std::cerr << "[SZM FEA] Decomposition failed — "
                     "check BCs (matrix may be singular).\n";
        return false;
    }

    m_U = cg.solve(m_F);
    if (cg.info() != Eigen::Success) {
        std::cerr << "[SZM FEA] PCG solver failed to converge.\n";
        return false;
    }

    // Write displacements back to nodes
    for (auto& node : m_Mesh.Nodes) {
        int dof = static_cast<int>(node.ID * 3);
        node.Displacement = Vec3d(m_U(dof), m_U(dof+1), m_U(dof+2));
    }
    std::cout << "[SZM FEA] Linear static solve complete. "
              << "Iterations: " << cg.iterations()
              << " | Error: " << cg.error() << "\n";
    return true;
}

// -------------------------------------------- CalculateStresses -----
void LinearStaticSolver::CalculateStresses() {
    for (auto& el : m_Mesh.Elements) {
        auto optMat = SZM::Materials::MaterialDatabase::GetInstance()
                          .GetMaterial(el.MaterialID);
        if (!optMat) continue;

        const auto& mat = optMat.value();
        // GPa → Pa for SI consistency
        Mat6d D = ComputeConstitutiveMatrix(mat.youngsModulus * 1e9,
                                             mat.poissonsRatio);

        // ε = B·u_local  (B-matrix integration pending 320-Series engineer)
        // σ = D·ε
        el.StressTensor = D * el.StrainTensor;

        // Von Mises stress
        double sxx = el.StressTensor(0), syy = el.StressTensor(1);
        double szz = el.StressTensor(2), txy = el.StressTensor(3);
        double tyz = el.StressTensor(4), tzx = el.StressTensor(5);

        double vm = std::sqrt(0.5 * (
            std::pow(sxx-syy, 2) + std::pow(syy-szz, 2) + std::pow(szz-sxx, 2) +
            6.0*(txy*txy + tyz*tyz + tzx*tzx)));

        // Scatter to nodes (averaged over connected elements in production)
        for (size_t nid : el.NodeIDs)
            m_Mesh.Nodes[nid].VonMisesStress = vm;
    }
}

// ---------------------------------------- ComputeConstitutiveMatrix -
Mat6d LinearStaticSolver::ComputeConstitutiveMatrix(double E, double nu) {
    Mat6d D = Mat6d::Zero();
    double f = E / ((1.0 + nu) * (1.0 - 2.0*nu));
    D(0,0)=D(1,1)=D(2,2) = f*(1.0-nu);
    D(0,1)=D(0,2)=D(1,0)=D(1,2)=D(2,0)=D(2,1) = f*nu;
    D(3,3)=D(4,4)=D(5,5) = f*(1.0-2.0*nu)/2.0;
    return D;
}

// ----------------------------------------- ComputeElementStiffness --
Eigen::MatrixXd LinearStaticSolver::ComputeElementStiffness(const FEAElement& el) {
    const size_t nodeCount = el.NodeIDs.size();
    const int dofs = static_cast<int>(nodeCount * 3);
    if (nodeCount < 4) {
        return Eigen::MatrixXd::Identity(std::max(dofs, 3), std::max(dofs, 3)) * 1e-9;
    }

    // Resolve material (fallback to steel-like isotropic material if missing).
    double E = 200e9;
    double nu = 0.30;
    if (auto mat = SZM::Materials::MaterialDatabase::GetInstance().GetMaterial(el.MaterialID)) {
        E = mat->youngsModulus * 1e9; // GPa -> Pa
        nu = mat->poissonsRatio;
    }
    const Mat6d D = ComputeConstitutiveMatrix(E, nu);

    // Tet4 constant-strain formulation on the first 4 nodes.
    const size_t n0 = el.NodeIDs[0];
    const size_t n1 = el.NodeIDs[1];
    const size_t n2 = el.NodeIDs[2];
    const size_t n3 = el.NodeIDs[3];
    if (n0 >= m_Mesh.Nodes.size() || n1 >= m_Mesh.Nodes.size() ||
        n2 >= m_Mesh.Nodes.size() || n3 >= m_Mesh.Nodes.size()) {
        return Eigen::MatrixXd::Identity(std::max(dofs, 12), std::max(dofs, 12)) * 1e-9;
    }

    const Vec3d& p0 = m_Mesh.Nodes[n0].Position;
    const Vec3d& p1 = m_Mesh.Nodes[n1].Position;
    const Vec3d& p2 = m_Mesh.Nodes[n2].Position;
    const Vec3d& p3 = m_Mesh.Nodes[n3].Position;

    Eigen::Matrix4d M;
    M << 1.0, p0.x(), p0.y(), p0.z(),
         1.0, p1.x(), p1.y(), p1.z(),
         1.0, p2.x(), p2.y(), p2.z(),
         1.0, p3.x(), p3.y(), p3.z();

    const double detM = M.determinant();
    const double volume = std::abs(detM) / 6.0;
    if (volume < 1e-12) {
        return Eigen::MatrixXd::Identity(std::max(dofs, 12), std::max(dofs, 12)) * 1e-9;
    }

    const Eigen::Matrix4d invM = M.inverse();
    Eigen::Matrix<double, 6, 12> B = Eigen::Matrix<double, 6, 12>::Zero();

    for (int i = 0; i < 4; ++i) {
        const double b = invM(1, i);
        const double c = invM(2, i);
        const double d = invM(3, i);
        const int col = i * 3;

        B(0, col + 0) = b;
        B(1, col + 1) = c;
        B(2, col + 2) = d;

        B(3, col + 0) = c;
        B(3, col + 1) = b;

        B(4, col + 1) = d;
        B(4, col + 2) = c;

        B(5, col + 0) = d;
        B(5, col + 2) = b;
    }

    Eigen::Matrix<double, 12, 12> ke4 = B.transpose() * D * B * volume;
    ke4 = 0.5 * (ke4 + ke4.transpose()); // Numerical symmetry cleanup

    // Expand into NxN (supports Tet10 fallback by stabilizing extra DOFs).
    Eigen::MatrixXd ke = Eigen::MatrixXd::Zero(dofs, dofs);
    ke.block<12, 12>(0, 0) = ke4;

    if (nodeCount > 4) {
        const double avgDiag = ke4.diagonal().mean();
        const double stabilizer = std::max(avgDiag, 1.0) * 1e-3;
        for (int i = 12; i < dofs; ++i) {
            ke(i, i) = stabilizer;
        }
    }

    return ke;
}

} // namespace SZM::FEA
