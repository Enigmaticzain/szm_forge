#pragma once

#include "KinematicTypes.hpp"

namespace SZM::Assembly {

    // ----------------------------------------------------------------
    //  CoincidentPlanarMate
    //  Constrains: 1 distance DOF + 2 orientation DOFs = 3 equations
    //
    //  Residuals:
    //    r[0] = (worldPB − worldPA) · worldNA         (point-on-plane)
    //    r[1] = (worldNA × worldNB).x
    //    r[2] = worldNA · worldNB - targetDot         (+1 aligned, -1 anti-aligned)
    // ----------------------------------------------------------------
    class CoincidentPlanarMate : public IMate3D {
    public:
        uint32_t         BodyA, BodyB;
        Eigen::Vector3d  LocalPointA,  LocalNormalA;
        Eigen::Vector3d  LocalPointB,  LocalNormalB;
        MateAlignment    Alignment;

        const RigidBodyState* StateA;
        const RigidBodyState* StateB;

        CoincidentPlanarMate(
            uint32_t bA, uint32_t bB,
            Eigen::Vector3d pA, Eigen::Vector3d nA,
            Eigen::Vector3d pB, Eigen::Vector3d nB,
            MateAlignment align,
            const RigidBodyState* sA,
            const RigidBodyState* sB)
            : BodyA(bA), BodyB(bB)
            , LocalPointA(std::move(pA)),  LocalNormalA(std::move(nA))
            , LocalPointB(std::move(pB)),  LocalNormalB(std::move(nB))
            , Alignment(align)
            , StateA(sA), StateB(sB) {}

        int GetConstraintCount() const override { return 3; }
        uint32_t GetBodyA_ID()  const override { return BodyA; }
        uint32_t GetBodyB_ID()  const override { return BodyB; }

        void EvaluateResidual(Eigen::VectorXd& residual, int row) const override {
            Eigen::Vector3d wPA = StateA->Transform * LocalPointA;
            Eigen::Vector3d wNA = StateA->Transform.rotation() * LocalNormalA;
            Eigen::Vector3d wPB = StateB->Transform * LocalPointB;
            Eigen::Vector3d wNB = StateB->Transform.rotation() * LocalNormalB;

            // 1. Distance: point B on plane A
            residual(row)     = (wPB - wPA).dot(wNA);

            // 2-3. Orientation: cross term + signed dot target.
            const double targetDot =
                (Alignment == MateAlignment::AntiAligned) ? -1.0 : 1.0;
            Eigen::Vector3d cross = wNA.cross(wNB);
            residual(row + 1) = cross.x();
            residual(row + 2) = wNA.dot(wNB) - targetDot;
        }

        void FillJacobian(Eigen::MatrixXd& J, int row,
                           int colA, int colB) const override
        {
            auto evalResidualForTransforms =
                [this](const Eigen::Isometry3d& tA, const Eigen::Isometry3d& tB) {
                    Eigen::Vector3d r = Eigen::Vector3d::Zero();
                    const Eigen::Vector3d wPA = tA * LocalPointA;
                    const Eigen::Vector3d wNA = tA.rotation() * LocalNormalA;
                    const Eigen::Vector3d wPB = tB * LocalPointB;
                    const Eigen::Vector3d wNB = tB.rotation() * LocalNormalB;

                    r(0) = (wPB - wPA).dot(wNA);
                    const Eigen::Vector3d cross = wNA.cross(wNB);
                    r(1) = cross.x();

                    const double targetDot =
                        (Alignment == MateAlignment::AntiAligned) ? -1.0 : 1.0;
                    r(2) = wNA.dot(wNB) - targetDot;
                    return r;
                };

            const Eigen::Isometry3d& tA = StateA->Transform;
            const Eigen::Isometry3d& tB = StateB->Transform;
            const Eigen::Vector3d baseResidual = evalResidualForTransforms(tA, tB);

            // Translational terms are analytic and stable.
            const Eigen::Vector3d wNA = tA.rotation() * LocalNormalA;
            if (colA >= 0) {
                J(row + 0, colA + 0) = -wNA.x();
                J(row + 0, colA + 1) = -wNA.y();
                J(row + 0, colA + 2) = -wNA.z();
            }
            if (colB >= 0) {
                J(row + 0, colB + 0) = wNA.x();
                J(row + 0, colB + 1) = wNA.y();
                J(row + 0, colB + 2) = wNA.z();
            }

            // Rotational terms are estimated via finite-difference in world axes.
            static constexpr double kEps = 1e-6;
            for (int axis = 0; axis < 3; ++axis) {
                Eigen::Vector3d worldAxis = Eigen::Vector3d::Zero();
                worldAxis(axis) = 1.0;
                const Eigen::Matrix3d dR =
                    Eigen::AngleAxisd(kEps, worldAxis).toRotationMatrix();

                if (colA >= 0) {
                    Eigen::Isometry3d pertA = tA;
                    pertA.linear() = dR * pertA.linear();
                    const Eigen::Vector3d pertResidual =
                        evalResidualForTransforms(pertA, tB);
                    const Eigen::Vector3d deriv = (pertResidual - baseResidual) / kEps;

                    J(row + 0, colA + 3 + axis) = deriv(0);
                    J(row + 1, colA + 3 + axis) = deriv(1);
                    J(row + 2, colA + 3 + axis) = deriv(2);
                }

                if (colB >= 0) {
                    Eigen::Isometry3d pertB = tB;
                    pertB.linear() = dR * pertB.linear();
                    const Eigen::Vector3d pertResidual =
                        evalResidualForTransforms(tA, pertB);
                    const Eigen::Vector3d deriv = (pertResidual - baseResidual) / kEps;

                    J(row + 0, colB + 3 + axis) = deriv(0);
                    J(row + 1, colB + 3 + axis) = deriv(1);
                    J(row + 2, colB + 3 + axis) = deriv(2);
                }
            }
        }
    };

    // ----------------------------------------------------------------
    //  ConcentricMate  (4 constraints)
    //  Locks:
    //    - radial offset between axes (2 translational constraints)
    //    - relative axis tilt (2 orientational constraints)
    //
    //  Leaves free:
    //    - translation along common axis
    //    - rotation around common axis
    // ----------------------------------------------------------------
    class ConcentricMate : public IMate3D {
    public:
        uint32_t         BodyA, BodyB;
        Eigen::Vector3d  LocalPointA, LocalAxisA;
        Eigen::Vector3d  LocalPointB, LocalAxisB;
        const RigidBodyState* StateA;
        const RigidBodyState* StateB;

        ConcentricMate(
            uint32_t bA, uint32_t bB,
            Eigen::Vector3d pA, Eigen::Vector3d axisA,
            Eigen::Vector3d pB, Eigen::Vector3d axisB,
            const RigidBodyState* sA,
            const RigidBodyState* sB)
            : BodyA(bA), BodyB(bB)
            , LocalPointA(std::move(pA)), LocalAxisA(std::move(axisA))
            , LocalPointB(std::move(pB)), LocalAxisB(std::move(axisB))
            , StateA(sA), StateB(sB) {}

        int GetConstraintCount() const override { return 4; }
        uint32_t GetBodyA_ID()  const override { return BodyA; }
        uint32_t GetBodyB_ID()  const override { return BodyB; }

        void EvaluateResidual(Eigen::VectorXd& residual, int row) const override {
            const Eigen::Vector3d wPA = StateA->Transform * LocalPointA;
            const Eigen::Vector3d wPB = StateB->Transform * LocalPointB;

            Eigen::Vector3d wAxisA = StateA->Transform.rotation() * LocalAxisA;
            Eigen::Vector3d wAxisB = StateB->Transform.rotation() * LocalAxisB;
            if (wAxisA.norm() < 1e-12 || wAxisB.norm() < 1e-12) {
                residual.segment<4>(row).setZero();
                return;
            }
            wAxisA.normalize();
            wAxisB.normalize();

            Eigen::Vector3d basisU = wAxisA.unitOrthogonal();
            Eigen::Vector3d basisV = wAxisA.cross(basisU).normalized();
            const Eigen::Vector3d d = wPB - wPA;

            // Radial coincidence in plane orthogonal to axisA.
            residual(row + 0) = d.dot(basisU);
            residual(row + 1) = d.dot(basisV);

            // Axis alignment in the same orthogonal basis.
            const Eigen::Vector3d axisMisalign = wAxisA.cross(wAxisB);
            residual(row + 2) = axisMisalign.dot(basisU);
            residual(row + 3) = axisMisalign.dot(basisV);
        }

        void FillJacobian(Eigen::MatrixXd& J, int row,
                          int colA, int colB) const override
        {
            auto evalResidualForTransforms =
                [this](const Eigen::Isometry3d& tA, const Eigen::Isometry3d& tB) {
                    Eigen::Matrix<double, 4, 1> r = Eigen::Matrix<double, 4, 1>::Zero();
                    const Eigen::Vector3d wPA = tA * LocalPointA;
                    const Eigen::Vector3d wPB = tB * LocalPointB;

                    Eigen::Vector3d wAxisA = tA.rotation() * LocalAxisA;
                    Eigen::Vector3d wAxisB = tB.rotation() * LocalAxisB;
                    if (wAxisA.norm() < 1e-12 || wAxisB.norm() < 1e-12) {
                        return r;
                    }
                    wAxisA.normalize();
                    wAxisB.normalize();

                    const Eigen::Vector3d basisU = wAxisA.unitOrthogonal();
                    const Eigen::Vector3d basisV = wAxisA.cross(basisU).normalized();
                    const Eigen::Vector3d d = wPB - wPA;

                    r(0) = d.dot(basisU);
                    r(1) = d.dot(basisV);

                    const Eigen::Vector3d axisMisalign = wAxisA.cross(wAxisB);
                    r(2) = axisMisalign.dot(basisU);
                    r(3) = axisMisalign.dot(basisV);
                    return r;
                };

            const Eigen::Isometry3d& tA = StateA->Transform;
            const Eigen::Isometry3d& tB = StateB->Transform;
            const Eigen::Matrix<double, 4, 1> baseResidual = evalResidualForTransforms(tA, tB);

            // Analytic translational jacobian for radial equations.
            Eigen::Vector3d wAxisA = tA.rotation() * LocalAxisA;
            if (wAxisA.norm() > 1e-12) {
                wAxisA.normalize();
                const Eigen::Vector3d basisU = wAxisA.unitOrthogonal();
                const Eigen::Vector3d basisV = wAxisA.cross(basisU).normalized();
                if (colA >= 0) {
                    J(row + 0, colA + 0) = -basisU.x();
                    J(row + 0, colA + 1) = -basisU.y();
                    J(row + 0, colA + 2) = -basisU.z();
                    J(row + 1, colA + 0) = -basisV.x();
                    J(row + 1, colA + 1) = -basisV.y();
                    J(row + 1, colA + 2) = -basisV.z();
                }
                if (colB >= 0) {
                    J(row + 0, colB + 0) = basisU.x();
                    J(row + 0, colB + 1) = basisU.y();
                    J(row + 0, colB + 2) = basisU.z();
                    J(row + 1, colB + 0) = basisV.x();
                    J(row + 1, colB + 1) = basisV.y();
                    J(row + 1, colB + 2) = basisV.z();
                }
            }

            static constexpr double kEps = 1e-6;
            for (int axis = 0; axis < 3; ++axis) {
                Eigen::Vector3d worldAxis = Eigen::Vector3d::Zero();
                worldAxis(axis) = 1.0;
                const Eigen::Matrix3d dR =
                    Eigen::AngleAxisd(kEps, worldAxis).toRotationMatrix();

                if (colA >= 0) {
                    Eigen::Isometry3d pertA = tA;
                    pertA.linear() = dR * pertA.linear();
                    const auto pertResidual = evalResidualForTransforms(pertA, tB);
                    const auto deriv = (pertResidual - baseResidual) / kEps;
                    for (int r = 0; r < 4; ++r) {
                        J(row + r, colA + 3 + axis) = deriv(r);
                    }
                }

                if (colB >= 0) {
                    Eigen::Isometry3d pertB = tB;
                    pertB.linear() = dR * pertB.linear();
                    const auto pertResidual = evalResidualForTransforms(tA, pertB);
                    const auto deriv = (pertResidual - baseResidual) / kEps;
                    for (int r = 0; r < 4; ++r) {
                        J(row + r, colB + 3 + axis) = deriv(r);
                    }
                }
            }
        }
    };

} // namespace SZM::Assembly
