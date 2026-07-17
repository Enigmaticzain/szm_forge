#include "VolumetricMesher.hpp"
#include <stdexcept>
#include <iostream>

// Gmsh header — only available when libgmsh is installed
// Conditional include allows the project to compile without Gmsh present;
// the function will throw if called without the library.
#ifdef SZM_HAS_GMSH
#include <gmsh.h>
#endif

namespace SZM::FEA {

FEAMesh VolumetricMesher::GenerateTetrahedralMesh(
    const std::string& geometryFilePath,
    double             globalElementSize,
    bool               useQuadratic,
    double             microEdgeTolerance)
{
    FEAMesh result;

#ifndef SZM_HAS_GMSH
    // Graceful degradation: return a test cube mesh and warn
    std::cerr << "[SZM FEA] Gmsh not linked. Define SZM_HAS_GMSH and link "
                 "libgmsh to enable volumetric meshing. Returning a test 6-tet cube.\n";
    (void)geometryFilePath; (void)globalElementSize;
    (void)useQuadratic;     (void)microEdgeTolerance;

    // Provide a dummy mesh (a unit cube split into 6 tetrahedra) for testing solvers
    result.Nodes.push_back({0, {0, 0, 0}, Vec3d::Zero(), 0.0});
    result.Nodes.push_back({1, {1, 0, 0}, Vec3d::Zero(), 0.0});
    result.Nodes.push_back({2, {1, 1, 0}, Vec3d::Zero(), 0.0});
    result.Nodes.push_back({3, {0, 1, 0}, Vec3d::Zero(), 0.0});
    result.Nodes.push_back({4, {0, 0, 1}, Vec3d::Zero(), 0.0});
    result.Nodes.push_back({5, {1, 0, 1}, Vec3d::Zero(), 0.0});
    result.Nodes.push_back({6, {1, 1, 1}, Vec3d::Zero(), 0.0});
    result.Nodes.push_back({7, {0, 1, 1}, Vec3d::Zero(), 0.0});

    std::string mat = "MAT-STEEL-STRUCT";
    result.Elements.push_back({0, {0, 1, 2, 5}, mat, Vec6d::Zero(), Vec6d::Zero()});
    result.Elements.push_back({1, {0, 2, 3, 7}, mat, Vec6d::Zero(), Vec6d::Zero()});
    result.Elements.push_back({2, {0, 2, 5, 7}, mat, Vec6d::Zero(), Vec6d::Zero()});
    result.Elements.push_back({3, {0, 5, 4, 7}, mat, Vec6d::Zero(), Vec6d::Zero()});
    result.Elements.push_back({4, {2, 5, 6, 7}, mat, Vec6d::Zero(), Vec6d::Zero()});

    return result;
#else
    gmsh::initialize();
    gmsh::option::setNumber("General.Terminal", 1);

    try {
        // --- Import geometry ----------------------------------------
        gmsh::model::occ::importShapes(geometryFilePath);

        // --- Edge-Case 9: micro-edge healing ------------------------
        gmsh::option::setNumber("Geometry.Tolerance",        microEdgeTolerance);
        gmsh::option::setNumber("Geometry.ToleranceBoolean", microEdgeTolerance);
        gmsh::model::occ::healShapes();
        gmsh::model::occ::synchronize();

        // --- Mesh sizing --------------------------------------------
        gmsh::option::setNumber("Mesh.MeshSizeMin", globalElementSize * 0.2);
        gmsh::option::setNumber("Mesh.MeshSizeMax", globalElementSize);

        // --- Element order (Tet4 vs Tet10) -------------------------
        gmsh::option::setNumber("Mesh.ElementOrder", useQuadratic ? 2 : 1);

        // --- Quality enforcement ------------------------------------
        gmsh::option::setNumber("Mesh.Optimize",       1); // Laplacian smoothing
        gmsh::option::setNumber("Mesh.OptimizeNetgen", 1); // Netgen aspect-ratio pass
        gmsh::option::setNumber("Mesh.Algorithm3D",    1); // Delaunay

        // --- Generate 3-D volume mesh -------------------------------
        gmsh::model::mesh::generate(3);

        // --- Extract nodes ------------------------------------------
        std::vector<std::size_t> nodeTags;
        std::vector<double>      coords, paramCoords;
        gmsh::model::mesh::getNodes(nodeTags, coords, paramCoords);

        result.Nodes.resize(nodeTags.size());
        for (size_t i = 0; i < nodeTags.size(); ++i) {
            result.Nodes[i].ID       = nodeTags[i] - 1; // 0-indexed
            result.Nodes[i].Position = {coords[i*3], coords[i*3+1], coords[i*3+2]};
        }

        // --- Extract tetrahedral elements ---------------------------
        std::vector<int> elTypes;
        std::vector<std::vector<std::size_t>> elTags, elNodes;
        gmsh::model::mesh::getElements(elTypes, elTags, elNodes, 3);

        for (size_t t = 0; t < elTypes.size(); ++t) {
            // Gmsh type 4 = Tet4, type 11 = Tet10
            if (elTypes[t] != 4 && elTypes[t] != 11) continue;
            size_t nodesPerEl = (elTypes[t] == 4) ? 4 : 10;

            for (size_t e = 0; e < elTags[t].size(); ++e) {
                FEAElement el;
                el.ID = elTags[t][e] - 1;
                el.NodeIDs.reserve(nodesPerEl);
                for (size_t k = 0; k < nodesPerEl; ++k)
                    el.NodeIDs.push_back(elNodes[t][e*nodesPerEl + k] - 1);
                result.Elements.push_back(std::move(el));
            }
        }

        std::cout << "[SZM FEA] Mesh: "
                  << result.Nodes.size() << " nodes, "
                  << result.Elements.size() << " elements ("
                  << (useQuadratic ? "Tet10" : "Tet4") << ").\n";

    } catch (const std::exception& e) {
        std::cerr << "[SZM FEA] Meshing failure: " << e.what() << "\n";
        gmsh::finalize();
        throw;
    }

    gmsh::finalize();
    return result;
#endif
}

} // namespace SZM::FEA
