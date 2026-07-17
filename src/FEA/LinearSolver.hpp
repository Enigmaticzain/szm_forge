#pragma once

#include "FEADataTypes.hpp"
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

namespace SZM::FEA {

    /**
     * @class LinearStaticSolver
     * @brief Linear static FEA solver using Eigen sparse PCG.
     *
     * Pipeline:
     *   1. AssembleSystem()      — builds global [K] and {F}
     *   2. ApplyBoundaryConditions() — diagonal-penalty method
     *   3. Solve()               — Preconditioned Conjugate Gradient
     *   4. CalculateStresses()   — σ = D·ε, Von Mises post-processing
     *
     * Materials are fetched from SZM::Materials::MaterialDatabase.
     */
    class LinearStaticSolver {
    public:
        explicit LinearStaticSolver(FEAMesh& mesh);
        ~LinearStaticSolver() = default;

        void AssembleSystem();
        void ApplyBoundaryConditions();
        bool Solve();
        void CalculateStresses();

    private:
        FEAMesh& m_Mesh;
        size_t   m_DOFCount = 0;

        Eigen::SparseMatrix<double> m_K;  ///< Global stiffness matrix
        Eigen::VectorXd             m_F;  ///< Global force vector
        Eigen::VectorXd             m_U;  ///< Global displacement solution

        /// Build 12×12 local stiffness matrix for one tetrahedron
        Eigen::MatrixXd ComputeElementStiffness(const FEAElement& el);

        /// 6×6 isotropic linear-elastic constitutive matrix
        Mat6d ComputeConstitutiveMatrix(double E, double nu);
    };

} // namespace SZM::FEA
