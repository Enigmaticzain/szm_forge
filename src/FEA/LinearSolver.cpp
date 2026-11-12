#include "LinearSolver.hpp"
#include "../Materials/MaterialDatabase.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace SZM::FEA {

namespace {
    bool ComputeTetStrainMatrix(const SZM::FEA::FEAMesh& mesh,
                                const SZM::FEA::FEAElement& el,
                                Eigen::Matrix<double, 6, 12>& outB,
                                double& outVolume)
    {
        if (el.NodeIDs.size() < 4) return false;
        const size_t n0 = el.NodeIDs[0], n1 = el.NodeIDs[1], n2 = el.NodeIDs[2], n3 = el.NodeIDs[3];
        if (n0 >= mesh.Nodes.size() || n1 >= mesh.Nodes.size() ||
            n2 >= mesh.Nodes.size() || n3 >= mesh.Nodes.size()) {
            return false;
        }

        const SZM::FEA::Vec3d& p0 = mesh.Nodes[n0].Position;
        const SZM::FEA::Vec3d& p1 = mesh.Nodes[n1].Position;
        const SZM::FEA::Vec3d& p2 = mesh.Nodes[n2].Position;
        const SZM::FEA::Vec3d& p3 = mesh.Nodes[n3].Position;

        Eigen::Matrix4d M;
        M << 1.0, p0.x(), p0.y(), p0.z(),
             1.0, p1.x(), p1.y(), p1.z(),
             1.0, p2.x(), p2.y(), p2.z(),
             1.0, p3.x(), p3.y(), p3.z();

        outVolume = std::abs(M.determinant()) / 6.0;
        if (outVolume < 1e-12) return false;

        const Eigen::Matrix4d invM = M.inverse();
        outB.setZero();
        for (int i = 0; i < 4; ++i) {
            const double b = invM(1, i);
            const double c = invM(2, i);
            const double d = invM(3, i);
            const int col = i * 3;

            outB(0, col + 0) = b;
            outB(1, col + 1) = c;
            outB(2, col + 2) = d;

            outB(3, col + 0) = c;
            outB(3, col + 1) = b;

            outB(4, col + 1) = d;
            outB(4, col + 2) = c;

            outB(5, col + 0) = d;
            outB(5, col + 2) = b;
        }
        return true;
    }
} // namespace

LinearStaticSolver::LinearStaticSolver(FEAMesh& mesh) : m_Mesh(mesh) {
    m_DOFCount = m_Mesh.Nodes.size() * 3;
    m_K.resize(static_cast<int>(m_DOFCount), static_cast<int>(m_DOFCount));
    m_F = Eigen::VectorXd::Zero(static_cast<int>(m_DOFCount));
    m_U = Eigen::VectorXd::Zero(static_cast<int>(m_DOFCount));
}

// ----------------------------------------------- AssembleSystem -----
void LinearStaticSolver::AssembleSystem() {
    m_F.setZero();
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(m_Mesh.Elements.size() * 144); // 12×12 per Tet

    // Assemble nodal force vector
    for (const auto& load : m_Mesh.Loads) {
        const size_t dof = static_cast<size_t>(load.NodeID) * 3;
        if (dof + 2 >= m_DOFCount) continue;
        m_F(static_cast<Eigen::Index>(dof))     += load.Force.x();
        m_F(static_cast<Eigen::Index>(dof + 1)) += load.Force.y();
        m_F(static_cast<Eigen::Index>(dof + 2)) += load.Force.z();
    }

    // Assemble global stiffness [K] via element scatter
    for (const auto& el : m_Mesh.Elements) {
        Eigen::MatrixXd ke = ComputeElementStiffness(el);
        for (size_t i = 0; i < el.NodeIDs.size(); ++i) {
            if (el.NodeIDs[i] >= m_Mesh.Nodes.size()) continue;
            for (size_t j = 0; j < el.NodeIDs.size(); ++j) {
                if (el.NodeIDs[j] >= m_Mesh.Nodes.size()) continue;
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
    if (m_DOFCount == 0) return;

    std::vector<bool> constrainedDofs(m_DOFCount, false);
    for (const auto& bc : m_Mesh.BCs) {
        if (bc.NodeID >= m_Mesh.Nodes.size()) continue;
        size_t base = static_cast<size_t>(bc.NodeID) * 3;
        if (base + 2 >= m_DOFCount) continue;
        if (bc.FixX) constrainedDofs[base + 0] = true;
        if (bc.FixY) constrainedDofs[base + 1] = true;
        if (bc.FixZ) constrainedDofs[base + 2] = true;
    }

    // Zero only existing sparse entries on constrained rows/columns without O(N^2) fill-in.
    for (int col = 0; col < m_K.outerSize(); ++col) {
        const bool colConstrained = constrainedDofs[static_cast<size_t>(col)];
        for (Eigen::SparseMatrix<double>::InnerIterator it(m_K, col); it; ++it) {
            const int row = static_cast<int>(it.row());
            if (colConstrained || constrainedDofs[static_cast<size_t>(row)]) {
                it.valueRef() = (row == col && colConstrained) ? 1.0 : 0.0;
            }
        }
    }

    for (size_t dof = 0; dof < m_DOFCount; ++dof) {
        if (!constrainedDofs[dof]) continue;
        const auto d = static_cast<Eigen::Index>(dof);
        if (m_K.coeff(d, d) == 0.0) {
            m_K.coeffRef(d, d) = 1.0;
        }
        m_F(d) = 0.0;
    }
    m_K.prune(0.0);
}

// ---------------------------------------------------------------- Solve
bool LinearStaticSolver::Solve() {
    if (m_DOFCount == 0) return false;

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
        const size_t dof = static_cast<size_t>(node.ID) * 3;
        if (dof + 2 >= m_DOFCount) continue;
        node.Displacement = Vec3d(m_U(static_cast<Eigen::Index>(dof)),
                                  m_U(static_cast<Eigen::Index>(dof + 1)),
                                  m_U(static_cast<Eigen::Index>(dof + 2)));
    }
    std::cout << "[SZM FEA] Linear static solve complete. "
              << "Iterations: " << cg.iterations()
              << " | Error: " << cg.error() << "\n";
    return true;
}

// -------------------------------------------- CalculateStresses -----
void LinearStaticSolver::CalculateStresses() {
    std::vector<double> nodalStressSum(m_Mesh.Nodes.size(), 0.0);
    std::vector<int> nodalElementCount(m_Mesh.Nodes.size(), 0);

    for (auto& node : m_Mesh.Nodes) {
        node.VonMisesStress = 0.0;
    }

    for (auto& el : m_Mesh.Elements) {
        auto optMat = SZM::Materials::MaterialDatabase::GetInstance()
                          .GetMaterial(el.MaterialID);
        double E = 200e9;
        double nu = 0.30;
        if (optMat) {
            E = optMat->youngsModulus * 1e9; // GPa -> Pa
            nu = optMat->poissonsRatio;
        }
        Mat6d D = ComputeConstitutiveMatrix(E, nu);

        Eigen::Matrix<double, 6, 12> B;
        double volume = 0.0;
        if (ComputeTetStrainMatrix(m_Mesh, el, B, volume)) {
            Eigen::Matrix<double, 12, 1> uEl = Eigen::Matrix<double, 12, 1>::Zero();
            for (size_t i = 0; i < 4; ++i) {
                const size_t dof = static_cast<size_t>(el.NodeIDs[i]) * 3;
                if (dof + 2 < m_DOFCount) {
                    uEl(static_cast<int>(i) * 3 + 0) = m_U(static_cast<Eigen::Index>(dof + 0));
                    uEl(static_cast<int>(i) * 3 + 1) = m_U(static_cast<Eigen::Index>(dof + 1));
                    uEl(static_cast<int>(i) * 3 + 2) = m_U(static_cast<Eigen::Index>(dof + 2));
                }
            }
            el.StrainTensor = B * uEl;
        }

        el.StressTensor = D * el.StrainTensor;

        double sxx = el.StressTensor(0), syy = el.StressTensor(1);
        double szz = el.StressTensor(2), txy = el.StressTensor(3);
        double tyz = el.StressTensor(4), tzx = el.StressTensor(5);

        double vm = std::sqrt(0.5 * (
            std::pow(sxx-syy, 2) + std::pow(syy-szz, 2) + std::pow(szz-sxx, 2) +
            6.0*(txy*txy + tyz*tyz + tzx*tzx)));

        for (size_t nid : el.NodeIDs) {
            if (nid < m_Mesh.Nodes.size()) {
                nodalStressSum[nid] += vm;
                nodalElementCount[nid] += 1;
            }
        }
    }

    for (size_t i = 0; i < m_Mesh.Nodes.size(); ++i) {
        if (nodalElementCount[i] > 0) {
            m_Mesh.Nodes[i].VonMisesStress = nodalStressSum[i] / nodalElementCount[i];
        }
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

    double E = 200e9;
    double nu = 0.30;
    if (auto mat = SZM::Materials::MaterialDatabase::GetInstance().GetMaterial(el.MaterialID)) {
        E = mat->youngsModulus * 1e9; // GPa -> Pa
        nu = mat->poissonsRatio;
    }
    const Mat6d D = ComputeConstitutiveMatrix(E, nu);

    Eigen::Matrix<double, 6, 12> B;
    double volume = 0.0;
    if (!ComputeTetStrainMatrix(m_Mesh, el, B, volume)) {
        return Eigen::MatrixXd::Identity(std::max(dofs, 12), std::max(dofs, 12)) * 1e-9;
    }

    Eigen::Matrix<double, 12, 12> ke4 = B.transpose() * D * B * volume;
    ke4 = 0.5 * (ke4 + ke4.transpose());

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
