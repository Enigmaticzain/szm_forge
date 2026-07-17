#pragma once

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <memory>
#include <vector>
#include <cstdint>

namespace SZM::Assembly {

    // ----------------------------------------------------------------
    //  RigidBodyState — 6-DOF pose tracked by the kinematic solver
    // ----------------------------------------------------------------
    struct RigidBodyState {
        uint32_t            ID;
        bool                IsGrounded = false;
        Eigen::Isometry3d   Transform  = Eigen::Isometry3d::Identity();

        RigidBodyState(uint32_t id, bool grounded = false)
            : ID(id), IsGrounded(grounded) {}
    };

    enum class MateAlignment {
        Aligned,     ///< Normals point in the same direction
        AntiAligned  ///< Normals point in opposite directions
    };

    // ----------------------------------------------------------------
    //  IMate3D — abstract interface for all 3-D assembly mates
    // ----------------------------------------------------------------
    class IMate3D {
    public:
        virtual ~IMate3D() = default;

        /// Number of scalar constraint equations this mate introduces
        virtual int GetConstraintCount() const = 0;

        /// Compute the residual error vector for the current body poses
        virtual void EvaluateResidual(Eigen::VectorXd& residual, int startRow) const = 0;

        /// Fill the Jacobian block: ∂residual/∂(body DOFs)
        virtual void FillJacobian(Eigen::MatrixXd& J, int startRow,
                                   int colIdxA, int colIdxB) const = 0;

        virtual uint32_t GetBodyA_ID() const = 0;
        virtual uint32_t GetBodyB_ID() const = 0;
    };

} // namespace SZM::Assembly
