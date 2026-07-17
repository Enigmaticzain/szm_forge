#include "core/fea/LinearSolver.hpp"
#include <iostream>
#include <cmath>
#include <vector>

namespace szm {
namespace fea {

LinearStaticSolver::LinearStaticSolver(FEAMesh& mesh) : m_mesh(mesh) {
    m_dofCount = m_mesh.nodes.size() * 3;
    m_globalStiffness.resize(m_dofCount, m_dofCount);
    m_globalForce = Eigen::VectorXd::Zero(m_dofCount);
    m_globalDisplacement = Eigen::VectorXd::Zero(m_dofCount);
}

void LinearStaticSolver::assembleSystem() {
    // Eigen Triplet list is the fastest way to build a sparse matrix
    std::vector<Eigen::Triplet<double>> tripletList;
    tripletList.reserve(m_mesh.elements.size() * 144); // 12x12 = 144 entries per Tet element

    // 1. Assemble Forces
    for (const auto& load : m_mesh.loads) {
        size_t dofIndex = load.nodeId * 3;
        m_globalForce(dofIndex) += load.force.x();
        m_globalForce(dofIndex + 1) += load.force.y();
        m_globalForce(dofIndex + 2) += load.force.z();
    }

    // 2. Assemble Global Stiffness Matrix [K]
    for (const auto& el : m_mesh.elements) {
        Eigen::MatrixXd ke = computeElementStiffness(el);
        
        // Scatter local ke into global [K]
        for (size_t i = 0; i < el.nodeIds.size(); ++i) {
            for (size_t j = 0; j < el.nodeIds.size(); ++j) {
                size_t globalRowNode = el.nodeIds[i];
                size_t globalColNode = el.nodeIds[j];
                
                for (int d1 = 0; d1 < 3; ++d1) {
                    for (int d2 = 0; d2 < 3; ++d2) {
                        double val = ke(i * 3 + d1, j * 3 + d2);
                        tripletList.emplace_back(globalRowNode * 3 + d1, globalColNode * 3 + d2, val);
                    }
                }
            }
        }
    }
    
    // Compress triplets into the sparse matrix
    m_globalStiffness.setFromTriplets(tripletList.begin(), tripletList.end());
}

void LinearStaticSolver::applyBoundaryConditions() {
    // Penalty method or Row/Col zeroing for symmetric matrices
    // Here we use a highly stable diagonal-modification approach for Conjugate Gradient
    
    for (const auto& bc : m_mesh.bcs) {
        size_t baseDof = bc.nodeId * 3;
        
        for (int i = 0; i < 3; ++i) {
            bool isFixed = (i == 0 && bc.fixX) || (i == 1 && bc.fixY) || (i == 2 && bc.fixZ);
            if (isFixed) {
                size_t dof = baseDof + i;
                
                // Zero out the row and column in the sparse matrix
                for (Eigen::SparseMatrix<double>::InnerIterator it(m_globalStiffness, dof); it; ++it) {
                    it.valueRef() = 0.0;
                }
                
                // Set diagonal to 1.0 to prevent singular matrix
                m_globalStiffness.coeffRef(dof, dof) = 1.0;
                
                // Force displacement at this node to be 0
                m_globalForce(dof) = 0.0; 
            }
        }
    }
}

bool LinearStaticSolver::solve() {
    // Use Preconditioned Conjugate Gradient for large, sparse, symmetric positive-definite matrices
    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper> cg;
    
    cg.compute(m_globalStiffness);
    if(cg.info() != Eigen::Success) {
        std::cerr << "SZM-CEA Error: Decomposition failed. Matrix may be singular (check BCs).\n";
        return false;
    }

    m_globalDisplacement = cg.solve(m_globalForce);
    
    if(cg.info() != Eigen::Success) {
        std::cerr << "SZM-CEA Error: Solving failed.\n";
        return false;
    }

    // Write results back to the mesh
    for (auto& node : m_mesh.nodes) {
        size_t dof = node.id * 3;
        node.displacement = Vector3d(m_globalDisplacement(dof), 
                                     m_globalDisplacement(dof+1), 
                                     m_globalDisplacement(dof+2));
    }
    return true;
}

void LinearStaticSolver::calculateStresses() {
    for (auto& el : m_mesh.elements) {
        // 1. Retrieve Material
        auto optMat = szm::materials::MaterialDatabase::getInstance().getMaterial(el.materialId);
        if (!optMat) continue;
        const auto& mat = optMat.value();

        // 2. Compute Strain (B-matrix * nodal displacements) 
        // Note: B-matrix derivation depends on shape functions. Abstracted here for modularity.
        // el.strainTensor = B * u_local;
        
        // 3. Compute Stress (Constitutive Matrix * Strain)
        Matrix6d D = computeConstitutiveMatrix(mat.youngsModulus * 1e9, mat.poissonsRatio); // GPa to Pa
        el.stressTensor = D * el.strainTensor;

        // 4. Calculate Von Mises Stress
        double s_xx = el.stressTensor(0); double s_yy = el.stressTensor(1); double s_zz = el.stressTensor(2);
        double t_xy = el.stressTensor(3); double t_yz = el.stressTensor(4); double t_zx = el.stressTensor(5);

        double vm_squared = 0.5 * (std::pow(s_xx - s_yy, 2) + 
                                   std::pow(s_yy - s_zz, 2) + 
                                   std::pow(s_zz - s_xx, 2) + 
                                   6.0 * (t_xy*t_xy + t_yz*t_yz + t_zx*t_zx));
                                   
        double vonMises = std::sqrt(vm_squared);
        
        // Map stress back to nodes for the UI heatmap averaging
        for(size_t nId : el.nodeIds) {
            // In a full implementation, you'd average the stress from all connected elements at this node
            m_mesh.nodes[nId].vonMisesStress = vonMises; 
        }
    }
}

Eigen::Matrix<double, 6, 6> LinearStaticSolver::computeConstitutiveMatrix(double E, double nu) {
    Matrix6d D = Matrix6d::Zero();
    double factor = E / ((1.0 + nu) * (1.0 - 2.0 * nu));

    D(0, 0) = D(1, 1) = D(2, 2) = factor * (1.0 - nu);
    D(0, 1) = D(0, 2) = D(1, 0) = D(1, 2) = D(2, 0) = D(2, 1) = factor * nu;
    D(3, 3) = D(4, 4) = D(5, 5) = factor * ((1.0 - 2.0 * nu) / 2.0); // Shear components

    return D;
}

Eigen::MatrixXd LinearStaticSolver::computeElementStiffness(const Element& el) {
    // Placeholder for geometric Jacobian and B-Matrix integration.
    // In a production tetrahedral solver, this integrates the shape function derivatives 
    // over the volume of the element: \int B^T * D * B dV
    return Eigen::MatrixXd::Identity(12, 12); 
}

} // namespace fea
} // namespace szm

# CMakeLists.txt (Appended from previous module)

# Find Eigen3 (Header-only library)
find_package(Eigen3 3.4 REQUIRED NO_MODULE)

# Define the Solver Library
add_library(FEA_Solver STATIC
    src/core/fea/LinearSolver.cpp
)

# Include directories
target_include_directories(FEA_Solver PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link Eigen3 and the previous Material Database module
target_link_libraries(FEA_Solver 
    PUBLIC Eigen3::Eigen 
    PRIVATE FEA_Materials
)

# Optional: Enable Intel MKL backend for Eigen if available on the system for max performance
option(USE_MKL "Use Intel MKL for Eigen backend" OFF)
if(USE_MKL)
    add_compile_definitions(EIGEN_USE_MKL_ALL)
    # Require linking to MKL libraries here
endif()

