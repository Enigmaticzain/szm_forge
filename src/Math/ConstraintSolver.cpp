#include "ConstraintSolver.hpp"
#include <iostream>
#include <algorithm>

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
        if (index < 0 || index >= static_cast<int>(m_Variables.size())) {
            throw std::out_of_range("ConstraintSolver2D variable index out of range");
        }
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
            F(i) = m_Constraints[i]->EvaluateResidual();

            Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numVars);
            m_Constraints[i]->FillJacobianRow(row);
            
            auto indices = m_Constraints[i]->GetVariableIndices();
            for (int idx : indices) {
                if (idx >= 0 && idx < numVars && m_Variables[idx].IsLocked) {
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
                return true;
            }

            Eigen::BDCSVD<Eigen::MatrixXd> svd(J, Eigen::ComputeThinU | Eigen::ComputeThinV);
            
            int activeVariables = 0;
            for(const auto& v : m_Variables) if(!v.IsLocked) activeVariables++;
            
            const Eigen::Index rank = svd.rank();
            m_DegreesOfFreedom = std::max(0, activeVariables - static_cast<int>(rank));

            Eigen::VectorXd deltaX = svd.solve(-F);

            if (static_cast<int>(J.rows()) > activeVariables && (J * deltaX + F).norm() > tolerance && deltaX.norm() < 1e-12) {
                std::cerr << "[SZM-CEA Error] System is Over-Constrained.\n";
                return false; 
            }

            for (int i = 0; i < numVars; ++i) {
                if (!m_Variables[i].IsLocked) {
                    m_Variables[i].Value += deltaX(i);
                }
            }
        }
        
        std::cerr << "[SZM-CEA Warning] Solver failed to converge.\n";
        return false;
    }

} // namespace SZM::Math
