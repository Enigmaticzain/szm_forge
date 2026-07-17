#pragma once

#include "KinematicTypes.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

namespace SZM::Assembly {

    /**
     * @class KinematicSolver
     * @brief Newton-Raphson solver for 3-D assembly mates.
     *
     * Mirrors the architecture of ConstraintSolver2D (Module 060) but
     * operates on 6-DOF rigid bodies in 3-D space using Eigen::Isometry3d
     * for pose representation, avoiding Gimbal-Lock on rotation updates.
     *
     * Each ungrounded body contributes 6 columns to the Jacobian.
     * Each mate contributes N rows (N = GetConstraintCount()).
     *
     * System DOF = active columns − rank(J)
     */
    class KinematicSolver {
    public:
        KinematicSolver() = default;

        void AddBody(std::shared_ptr<RigidBodyState> body);
        void AddMate(std::shared_ptr<IMate3D> mate);

        bool Solve(int maxIterations = 50, double tolerance = 1e-6);

        int GetSystemDOF() const { return m_SystemDOF; }

    private:
        std::vector<std::shared_ptr<RigidBodyState>> m_Bodies;
        std::vector<std::shared_ptr<IMate3D>>        m_Mates;
        std::unordered_map<uint32_t, int>            m_ColumnMap;

        int m_ActiveDOFCount   = 0;
        int m_TotalConstraints = 0;
        int m_SystemDOF        = 0;

        std::shared_ptr<RigidBodyState> FindBody(uint32_t id);
        void ApplyDeltas(const Eigen::VectorXd& delta);
    };

} // namespace SZM::Assembly
