#pragma once
#include "FEADataTypes.hpp"
#include "core/materials/MaterialDatabase.hpp"
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

namespace szm {
namespace fea {

class LinearStaticSolver {
public:
    LinearStaticSolver(FEAMesh& mesh);
    ~LinearStaticSolver() = default;

    // Core Pipeline
    void assembleSystem();
    void applyBoundaryConditions();
    bool solve();
    void calculateStresses();

private:
    FEAMesh& m_mesh;
    size_t m_dofCount; // Total Degrees of Freedom (Nodes * 3)

    // Global Matrices and Vectors
    Eigen::SparseMatrix<double> m_globalStiffness;
    Eigen::VectorXd m_globalForce;
    Eigen::VectorXd m_globalDisplacement;

    // Helper: Build the local 12x12 stiffness matrix for a single Tetrahedron
    Eigen::MatrixXd computeElementStiffness(const Element& el);
    
    // Helper: Compute the 6x6 material constitutive matrix (Isotropic Linear Elastic)
    Matrix6d computeConstitutiveMatrix(double E, double nu);
};

} // namespace fea
} // namespace szm

