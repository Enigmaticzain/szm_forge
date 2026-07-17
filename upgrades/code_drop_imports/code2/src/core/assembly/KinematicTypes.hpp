#pragma once
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <memory>
#include <vector>

namespace SZM::Assembly {

    // Represents a rigid body's state in the solver
    struct RigidBodyState {
        uint32_t ID;
        bool IsGrounded;
        
        // 4x4 Transformation Matrix
        Eigen::Isometry3d Transform; 

        RigidBodyState(uint32_t id, bool grounded = false) 
            : ID(id), IsGrounded(grounded), Transform(Eigen::Isometry3d::Identity()) {}
    };

    enum class MateAlignment {
        Aligned,     // Normals point in the same direction
        AntiAligned  // Normals point in opposite directions
    };

    // The interface for all 3D Mates
    class IMate3D {
    public:
        virtual ~IMate3D() = default;

        // Returns the number of constraint equations this mate generates 
        // (e.g., Coincident = 3, Concentric = 4)
        virtual int GetConstraintCount() const = 0;

        // Evaluates the current mathematical error of the mate
        virtual void EvaluateResidual(Eigen::VectorXd& residual, int startRow) const = 0;

        // Fills the Jacobian block for Body A and Body B
        virtual void FillJacobian(Eigen::MatrixXd& jacobian, int startRow, 
                                  int colIdxA, int colIdxB) const = 0;

        // Which bodies are involved?
        virtual uint32_t GetBodyA_ID() const = 0;
        virtual uint32_t GetBodyB_ID() const = 0;
    };

} // namespace SZM::Assembly

