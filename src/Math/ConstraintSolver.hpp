#pragma once

#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace SZM::Math {

    struct Variable {
        double Value;
        bool IsLocked;
    };

    class IConstraint {
    public:
        virtual ~IConstraint() = default;
        virtual double EvaluateResidual() const = 0;
        virtual void FillJacobianRow(Eigen::RowVectorXd& jacobianRow) const = 0;
        virtual std::vector<int> GetVariableIndices() const = 0;
    };

    class ConstraintSolver2D {
    public:
        ConstraintSolver2D();
        ~ConstraintSolver2D() = default;

        int AddVariable(double initialValue, bool locked = false);
        void AddConstraint(std::shared_ptr<IConstraint> constraint);

        bool Solve(int maxIterations = 50, double tolerance = 1e-6);

        double GetVariableValue(int index) const;
        int GetDegreesOfFreedom() const;
        double GetSystemResidualError() const;
        const std::vector<Variable>& GetVariables() const { return m_Variables; }
        std::vector<Variable>& GetVariables() { return m_Variables; }

    private:
        std::vector<Variable> m_Variables;
        std::vector<std::shared_ptr<IConstraint>> m_Constraints;
        
        int m_DegreesOfFreedom = 0;
        double m_ResidualError = 0.0;
        
        void BuildSystem(Eigen::MatrixXd& J, Eigen::VectorXd& F);
    };

} // namespace SZM::Math
