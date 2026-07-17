#pragma once
#include "KinematicTypes.hpp"

namespace SZM::Assembly {

    class CoincidentPlanarMate : public IMate3D {
    public:
        uint32_t BodyA, BodyB;
        Eigen::Vector3d LocalPointA, LocalNormalA;
        Eigen::Vector3d LocalPointB, LocalNormalB;
        MateAlignment Alignment;
        
        // Pointers to the live states in the solver
        const RigidBodyState* StateA;
        const RigidBodyState* StateB;

        CoincidentPlanarMate(uint32_t bA, uint32_t bB, 
                             Eigen::Vector3d pA, Eigen::Vector3d nA,
                             Eigen::Vector3d pB, Eigen::Vector3d nB,
                             MateAlignment align,
                             const RigidBodyState* sA, const RigidBodyState* sB)
            : BodyA(bA), BodyB(bB), LocalPointA(pA), LocalNormalA(nA), 
              LocalPointB(pB), LocalNormalB(nB), Alignment(align), StateA(sA), StateB(sB) {}

        int GetConstraintCount() const override { return 3; } // 1 Distance, 2 Orientation DOFs

        uint32_t GetBodyA_ID() const override { return BodyA; }
        uint32_t GetBodyB_ID() const override { return BodyB; }

        void EvaluateResidual(Eigen::VectorXd& residual, int startRow) const override {
            // Transform local geometry to world space using current state
            Eigen::Vector3d worldPA = StateA->Transform * LocalPointA;
            Eigen::Vector3d worldNA = StateA->Transform.rotation() * LocalNormalA;
            
            Eigen::Vector3d worldPB = StateB->Transform * LocalPointB;
            Eigen::Vector3d worldNB = StateB->Transform.rotation() * LocalNormalB;

            double targetDot = (Alignment == MateAlignment::AntiAligned) ? -1.0 : 1.0;

            // 1. Distance constraint: Point B must lie on Plane A
            residual(startRow) = (worldPB - worldPA).dot(worldNA);

            // 2. Orientation constraint: Cross product of normals should be 0
            Eigen::Vector3d cross = worldNA.cross(worldNB);
            // We only need 2 independent equations from the cross product
            residual(startRow + 1) = cross.x();
            residual(startRow + 2) = cross.y(); 
        }

        void FillJacobian(Eigen::MatrixXd& jacobian, int startRow, int colIdxA, int colIdxB) const override {
            // TODO: Calculate the partial derivatives of the distance and orientation
            // with respect to the 6 DOFs (dx, dy, dz, dtheta_x, dtheta_y, dtheta_z)
            // for both Body A and Body B. 
            // Insert the 3x6 block into jacobian.block(startRow, colIdxA, 3, 6)
        }
    };

} // namespace SZM::Assembly

