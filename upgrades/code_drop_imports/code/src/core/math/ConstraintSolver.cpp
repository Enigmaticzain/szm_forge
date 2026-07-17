#include "ConstraintSolver.hpp"
#include <iostream>

namespace SZM::Math {

    ConstraintSolver2D::ConstraintSolver2D() {}

    int ConstraintSolver2D::AddVariable(double initialValue, bool locked) {
        m_Variables.push_back({initialValue, locked});
        return m_Variables.size() - 1;
    }

    void ConstraintSolver2D::AddConstraint(std::shared_ptr<IConstraint> constraint) {
        m_Constraints.push_back(constraint);
    }

    double ConstraintSolver2D::GetVariableValue(int index) const {
        return m_Variables[index].Value;
    }

    int ConstraintSolver2D::GetDegreesOfFreedom() const {
        return m_DegreesOfFreedom;
    }

    double ConstraintSolver2D::GetSystemResidualError() const {
        return m_ResidualError;
    }

    void ConstraintSolver2D::BuildSystem(Eigen::MatrixXd& J, Eigen::VectorXd& F) {
        int numVars = m_Variables.size();
        int numConstraints = m_Constraints.size();

        J = Eigen::MatrixXd::Zero(numConstraints, numVars);
        F = Eigen::VectorXd::Zero(numConstraints);

        for (int i = 0; i < numConstraints; ++i) {
            // 1. Calculate Residual f(x)
            F(i) = m_Constraints[i]->EvaluateResidual();

            // 2. Calculate Jacobian Row J
            Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numVars);
            m_Constraints[i]->FillJacobianRow(row);
            
            // Mask out locked variables (they don't contribute to derivatives)
            auto indices = m_Constraints[i]->GetVariableIndices();
            for (int idx : indices) {
                if (m_Variables[idx].IsLocked) {
                    row(idx) = 0.0; 
                }
            }
            J.row(i) = row;
        }
    }

    bool ConstraintSolver2D::Solve(int maxIterations, double tolerance) {
        int numVars = m_Variables.size();
        if (numVars == 0) return true;

        for (int iter = 0; iter < maxIterations; ++iter) {
            Eigen::MatrixXd J;
            Eigen::VectorXd F;
            BuildSystem(J, F);

            m_ResidualError = F.norm();
            if (m_ResidualError < tolerance) {
                return true; // System successfully constrained and converged
            }

            // Solve J * delta_x = -F using Singular Value Decomposition
            // BDCSVD is highly optimized for larger matrices in Eigen
            Eigen::BDCSVD<Eigen::MatrixXd> svd(J, Eigen::ComputeThinU | Eigen::ComputeThinV);
            
            // Calculate actual Degrees of Freedom
            // Total independent variables minus the rank of the Jacobian
            int activeVariables = 0;
            for(const auto& v : m_Variables) if(!v.IsLocked) activeVariables++;
            
            m_DegreesOfFreedom = activeVariables - svd.rank();

            // Check for over-constraint failure condition
            if (m_DegreesOfFreedom < 0) {
                std::cerr << "[SZM-CEA Error] System is Over-Constrained. Conflicting geometry detected.\n";
                return false; 
            }

            Eigen::VectorXd deltaX = svd.solve(-F);

            // Apply the delta to our variables
            for (int i = 0; i < numVars; ++i) {
                if (!m_Variables[i].IsLocked) {
                    m_Variables[i].Value += deltaX(i);
                }
            }
        }
        
        std::cerr << "[SZM-CEA Warning] Solver failed to converge after " << maxIterations << " iterations.\n";
        return false;
    }

} // namespace SZM::Math

