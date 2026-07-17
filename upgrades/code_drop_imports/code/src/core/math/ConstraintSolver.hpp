#pragma once
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace SZM::Math {

    // Represents a single variable in the solver (e.g., a Point's X or Y coordinate)
    struct Variable {
        double Value;
        bool IsLocked; // True if anchored to the origin or explicitly fixed
    };

    // Base interface for all geometric constraints
    class IConstraint {
    public:
        virtual ~IConstraint() = default;

        // Evaluates how close the constraint is to being satisfied (Target: 0.0)
        virtual double EvaluateResidual() const = 0;

        // Fills the partial derivatives of this constraint with respect to its variables
        virtual void FillJacobianRow(Eigen::RowVectorXd& jacobianRow) const = 0;
        
        // Maps the constraint's local variables to the global solver indices
        virtual std::vector<int> GetVariableIndices() const = 0;
    };

    class ConstraintSolver2D {
    public:
        ConstraintSolver2D();
        ~ConstraintSolver2D() = default;

        int AddVariable(double initialValue, bool locked = false);
        void AddConstraint(std::shared_ptr<IConstraint> constraint);

        // Executes the Newton-Raphson loop. Returns true if converged.
        bool Solve(int maxIterations = 50, double tolerance = 1e-6);

        // State Accessors
        double GetVariableValue(int index) const;
        int GetDegreesOfFreedom() const;
        double GetSystemResidualError() const;

    private:
        std::vector<Variable> m_Variables;
        std::vector<std::shared_ptr<IConstraint>> m_Constraints;
        
        int m_DegreesOfFreedom = 0;
        double m_ResidualError = 0.0;
        
        // Helper to construct the system matrices for the current iteration
        void BuildSystem(Eigen::MatrixXd& J, Eigen::VectorXd& F);
    };

} // namespace SZM::Math

