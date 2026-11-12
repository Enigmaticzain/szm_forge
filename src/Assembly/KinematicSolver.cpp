#include "KinematicSolver.hpp"
#include <Eigen/Dense>
#include <iostream>

namespace SZM::Assembly {

// ---------------------------------------------------------------- AddBody
void KinematicSolver::AddBody(std::shared_ptr<RigidBodyState> body) {
    if (!body) return;
    m_Bodies.push_back(body);
    if (!body->IsGrounded) {
        m_ColumnMap[body->ID] = m_ActiveDOFCount;
        m_ActiveDOFCount += 6; // 6 DOFs per free body
    }
}

// ---------------------------------------------------------------- AddMate
void KinematicSolver::AddMate(std::shared_ptr<IMate3D> mate) {
    if (!mate) return;
    m_Mates.push_back(std::move(mate));
    m_TotalConstraints += m_Mates.back()->GetConstraintCount();
}

// -------------------------------------------------------------- FindBody
std::shared_ptr<RigidBodyState> KinematicSolver::FindBody(uint32_t id) {
    for (auto& b : m_Bodies)
        if (b && b->ID == id) return b;
    return nullptr;
}

// ------------------------------------------------------------- ApplyDeltas
void KinematicSolver::ApplyDeltas(const Eigen::VectorXd& delta) {
    for (auto& body : m_Bodies) {
        if (!body || body->IsGrounded) continue;
        auto it = m_ColumnMap.find(body->ID);
        if (it == m_ColumnMap.end()) continue;
        int col = it->second;
        if (col < 0 || col + 5 >= delta.size()) continue;

        // Translational update
        Eigen::Vector3d dT(delta(col), delta(col+1), delta(col+2));
        body->Transform.translation() += dT;

        // Rotational update — small-angle AngleAxis to avoid Gimbal Lock
        Eigen::Vector3d dR(delta(col+3), delta(col+4), delta(col+5));
        double angle = dR.norm();
        if (angle > 1e-8) {
            Eigen::AngleAxisd rot(angle, dR.normalized());
            body->Transform.linear() =
                rot.toRotationMatrix() * body->Transform.linear();
        }
    }
}

// ------------------------------------------------------------------ Solve
bool KinematicSolver::Solve(int maxIterations, double tolerance) {
    if (m_ActiveDOFCount == 0 || m_TotalConstraints == 0) return true;

    for (int iter = 0; iter < maxIterations; ++iter) {
        Eigen::MatrixXd J = Eigen::MatrixXd::Zero(m_TotalConstraints, m_ActiveDOFCount);
        Eigen::VectorXd F = Eigen::VectorXd::Zero(m_TotalConstraints);

        int row = 0;
        for (const auto& mate : m_Mates) {
            // Resolve body states by ID each iteration — never trust stored raw pointers alone
            auto bA = FindBody(mate->GetBodyA_ID());
            auto bB = FindBody(mate->GetBodyB_ID());
            mate->BindStates(bA.get(), bB.get());

            // Skip Jacobian fill if either body is unregistered (avoids negative-index UB)
            if (!bA || !bB) {
                row += mate->GetConstraintCount();
                continue;
            }

            mate->EvaluateResidual(F, row);

            int colA = -1, colB = -1;
            auto itA = m_ColumnMap.find(mate->GetBodyA_ID());
            auto itB = m_ColumnMap.find(mate->GetBodyB_ID());
            if (!bA->IsGrounded && itA != m_ColumnMap.end()) colA = itA->second;
            if (!bB->IsGrounded && itB != m_ColumnMap.end()) colB = itB->second;

            mate->FillJacobian(J, row, colA, colB);
            row += mate->GetConstraintCount();
        }

        if (F.norm() < tolerance) {
            m_SystemDOF = m_ActiveDOFCount - m_TotalConstraints;
            return true;
        }

        // SVD — handles under/over-constrained systems gracefully
        Eigen::BDCSVD<Eigen::MatrixXd> svd(J,
            Eigen::ComputeThinU | Eigen::ComputeThinV);

        if (svd.rank() < m_TotalConstraints &&
            m_TotalConstraints > m_ActiveDOFCount) {
            std::cerr << "[SZM Kinematics] Conflicting constraints detected — "
                         "rank-deficient Jacobian.\n";
            return false;
        }

        ApplyDeltas(svd.solve(-F));
    }

    std::cerr << "[SZM Kinematics] Assembly solver did not converge.\n";
    return false;
}

} // namespace SZM::Assembly
