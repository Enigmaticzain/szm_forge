#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace SZM::FEA {

    using Vec3d   = Eigen::Vector3d;
    using Mat3d   = Eigen::Matrix3d;
    using Mat6d   = Eigen::Matrix<double, 6, 6>; ///< Constitutive matrix
    using Vec6d   = Eigen::Matrix<double, 6, 1>; ///< Stress / strain vector

    // ----------------------------------------------------------------
    //  FEA Node — one spatial point with 3 displacement DOFs
    // ----------------------------------------------------------------
    struct FEANode {
        size_t  ID;
        Vec3d   Position;
        Vec3d   Displacement = Vec3d::Zero();
        double  VonMisesStress = 0.0;
    };

    // ----------------------------------------------------------------
    //  FEA Element — tetrahedron (Tet4 or Tet10)
    // ----------------------------------------------------------------
    struct FEAElement {
        size_t             ID;
        std::vector<size_t> NodeIDs;     ///< 4 (Tet4) or 10 (Tet10) node indices
        std::string        MaterialID;   ///< Lookup key in MaterialDatabase
        Vec6d              StressTensor = Vec6d::Zero();
        Vec6d              StrainTensor = Vec6d::Zero();
    };

    // ----------------------------------------------------------------
    //  Boundary Condition — locks specific DOFs at a node
    // ----------------------------------------------------------------
    struct BoundaryCondition {
        size_t NodeID;
        bool   FixX = false;
        bool   FixY = false;
        bool   FixZ = false;
    };

    // ----------------------------------------------------------------
    //  Nodal Load — point force [N] applied at a node
    // ----------------------------------------------------------------
    struct NodalLoad {
        size_t NodeID;
        Vec3d  Force = Vec3d::Zero(); ///< (Fx, Fy, Fz) [N]
    };

    // ----------------------------------------------------------------
    //  FEA Mesh — complete problem definition passed to solver
    // ----------------------------------------------------------------
    struct FEAMesh {
        std::vector<FEANode>            Nodes;
        std::vector<FEAElement>         Elements;
        std::vector<BoundaryCondition>  BCs;
        std::vector<NodalLoad>          Loads;
    };

} // namespace SZM::FEA
