#include "KinematicTypes.hpp"
#include <iostream>
#include <unordered_map>

namespace SZM::Assembly {

    class KinematicSolver {
    public:
        void AddBody(std::shared_ptr<RigidBodyState> body) {
            m_Bodies.push_back(body);
            if (!body->IsGrounded) {
                m_ColumnMap[body->ID] = m_ActiveDOFCount;
                m_ActiveDOFCount += 6; // 6 DOFs per ungrounded body
            }
        }

        void AddMate(std::shared_ptr<IMate3D> mate) {
            m_Mates.push_back(mate);
            m_TotalConstraints += mate->GetConstraintCount();
        }

        bool Solve(int maxIterations = 50, double tolerance = 1e-6) {
            if (m_ActiveDOFCount == 0 || m_TotalConstraints == 0) return true;

            for (int iter = 0; iter < maxIterations; ++iter) {
                Eigen::MatrixXd J = Eigen::MatrixXd::Zero(m_TotalConstraints, m_ActiveDOFCount);
                Eigen::VectorXd F = Eigen::VectorXd::Zero(m_TotalConstraints);

                int currentRow = 0;
                for (const auto& mate : m_Mates) {
                    mate->EvaluateResidual(F, currentRow);

                    int colA = -1, colB = -1;
                    if (!GetBody(mate->GetBodyA_ID())->IsGrounded) colA = m_ColumnMap[mate->GetBodyA_ID()];
                    if (!GetBody(mate->GetBodyB_ID())->IsGrounded) colB = m_ColumnMap[mate->GetBodyB_ID()];

                    mate->FillJacobian(J, currentRow, colA, colB);
                    currentRow += mate->GetConstraintCount();
                }

                // Check convergence
                if (F.norm() < tolerance) {
                    m_SystemDOF = m_ActiveDOFCount - m_TotalConstraints; // Simplified DOF calculation
                    return true; 
                }

                // SVD Solver to handle under/over-constrained systems
                Eigen::BDCSVD<Eigen::MatrixXd> svd(J, Eigen::ComputeThinU | Eigen::ComputeThinV);
                
                // Rank check for failure condition (Requirement 9: Conflicting constraints)
                if (svd.rank() < m_TotalConstraints && m_TotalConstraints > m_ActiveDOFCount) {
                    std::cerr << "[SZM-Kinematics Error] Conflicting constraints detected. Rank deficient Jacobian.\n";
                    return false;
                }

                Eigen::VectorXd delta = svd.solve(-F);

                // Apply spatial updates
                ApplyDeltas(delta);
            }
            
            std::cerr << "[SZM-Kinematics Warning] Assembly failed to converge.\n";
            return false;
        }

        int GetSystemDOF() const { return m_SystemDOF; }

    private:
        std::vector<std::shared_ptr<RigidBodyState>> m_Bodies;
        std::vector<std::shared_ptr<IMate3D>> m_Mates;
        std::unordered_map<uint32_t, int> m_ColumnMap;
        
        int m_ActiveDOFCount = 0;
        int m_TotalConstraints = 0;
        int m_SystemDOF = 0;

        std::shared_ptr<RigidBodyState> GetBody(uint32_t id) {
            for (auto& b : m_Bodies) if (b->ID == id) return b;
            return nullptr;
        }

        void ApplyDeltas(const Eigen::VectorXd& delta) {
            for (auto& body : m_Bodies) {
                if (body->IsGrounded) continue;
                
                int colStart = m_ColumnMap[body->ID];
                
                // Extract translational delta (dx, dy, dz)
                Eigen::Vector3d dTranslation(delta(colStart), delta(colStart+1), delta(colStart+2));
                
                // Extract rotational delta (using small-angle approximation for update)
                Eigen::Vector3d dRotation(delta(colStart+3), delta(colStart+4), delta(colStart+5));
                
                // Update transformation matrix
                body->Transform.translation() += dTranslation;
                
                // Apply rotation update using an AngleAxis
                double angle = dRotation.norm();
                if (angle > 1e-8) {
                    Eigen::AngleAxisd updateRot(angle, dRotation.normalized());
                    body->Transform.linear() = updateRot.toRotationMatrix() * body->Transform.linear();
                }
            }
        }
    };

} // namespace SZM::Assembly

# src/core/assembly/CMakeLists.txt

find_package(Eigen3 3.4 REQUIRED NO_MODULE)

add_library(SZM_Assembly_Module STATIC
    KinematicSolver.cpp
)

target_link_libraries(SZM_Assembly_Module PUBLIC Eigen3::Eigen)

target_include_directories(SZM_Assembly_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(SZM_Assembly_Module PUBLIC cxx_std_20)

