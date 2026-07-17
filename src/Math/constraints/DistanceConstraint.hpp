#pragma once

#include "../ConstraintSolver.hpp"
#include <cmath>

namespace SZM::Math {

    /**
     * @class DistanceConstraint
     * @brief Enforces a fixed Euclidean distance between two 2-D points.
     *
     * Residual:  f(x) = (x1−x2)² + (y1−y2)² − D²
     * Jacobian:
     *   ∂f/∂x1 =  2(x1−x2)    ∂f/∂x2 = −2(x1−x2)
     *   ∂f/∂y1 =  2(y1−y2)    ∂f/∂y2 = −2(y1−y2)
     */
    class DistanceConstraint : public IConstraint {
    public:
        int idx_X1, idx_Y1, idx_X2, idx_Y2;
        double TargetDistance;
        std::vector<Variable>& GlobalVariables; ///< Reference into the solver's list

        DistanceConstraint(int x1, int y1, int x2, int y2,
                           double distance,
                           std::vector<Variable>& vars)
            : idx_X1(x1), idx_Y1(y1), idx_X2(x2), idx_Y2(y2)
            , TargetDistance(distance)
            , GlobalVariables(vars) {}

        double EvaluateResidual() const override {
            double dx = GlobalVariables[idx_X1].Value - GlobalVariables[idx_X2].Value;
            double dy = GlobalVariables[idx_Y1].Value - GlobalVariables[idx_Y2].Value;
            return dx*dx + dy*dy - TargetDistance*TargetDistance;
        }

        void FillJacobianRow(Eigen::RowVectorXd& row) const override {
            double dx = GlobalVariables[idx_X1].Value - GlobalVariables[idx_X2].Value;
            double dy = GlobalVariables[idx_Y1].Value - GlobalVariables[idx_Y2].Value;
            row(idx_X1) =  2.0 * dx;
            row(idx_X2) = -2.0 * dx;
            row(idx_Y1) =  2.0 * dy;
            row(idx_Y2) = -2.0 * dy;
        }

        std::vector<int> GetVariableIndices() const override {
            return { idx_X1, idx_Y1, idx_X2, idx_Y2 };
        }
    };

} // namespace SZM::Math
