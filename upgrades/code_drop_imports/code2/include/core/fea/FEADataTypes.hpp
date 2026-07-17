#pragma once
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace szm {
namespace fea {

// Using Eigen's fast math types
using Vector3d = Eigen::Vector3d;
using Matrix3d = Eigen::Matrix3d;
using Matrix6d = Eigen::Matrix<double, 6, 6>; // 6x6 Constitutive matrix

struct Node {
    size_t id;
    Vector3d position;
    Vector3d displacement{0.0, 0.0, 0.0};
    double vonMisesStress{0.0};
};

struct Element {
    size_t id;
    std::vector<size_t> nodeIds; // e.g., 4 nodes for a Tetrahedron
    std::string materialId;
    
    // Tensors
    Eigen::Matrix<double, 6, 1> stressTensor; 
    Eigen::Matrix<double, 6, 1> strainTensor;
};

struct BoundaryCondition {
    size_t nodeId;
    bool fixX, fixY, fixZ; // True if locked in that Degree of Freedom (DOF)
};

struct NodalLoad {
    size_t nodeId;
    Vector3d force; // Force in Newtons (Fx, Fy, Fz)
};

struct FEAMesh {
    std::vector<Node> nodes;
    std::vector<Element> elements;
    std::vector<BoundaryCondition> bcs;
    std::vector<NodalLoad> loads;
};

} // namespace fea
} // namespace szm

