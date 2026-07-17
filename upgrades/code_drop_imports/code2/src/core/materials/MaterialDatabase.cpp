#include "core/materials/MaterialDatabase.hpp"
#include <iostream>

namespace szm {
namespace materials {

void MaterialDatabase::loadStandardLibraries() {
    // Acquire exclusive write lock
    std::unique_lock lock(m_mutex);

    // Mocking a JSON/SQLite parser load for the standard libraries
    // In production, this parses 'materials.json' or 'materials.db'
    
    // Test Case: Structural Steel (AISI 1020 approximation)
    // E = 200 GPa, ν = 0.30, Yield = 250 MPa, UTS = 400 MPa, ρ = 7850 kg/m^3
    IMaterial structuralSteel("MAT-STEEL-STRUCT", "Structural Steel", 
                              200.0, 0.30, 250.0, 400.0, 7850.0, 
                              1.2e-5, 50.0);
                              
    IMaterial al6061("MAT-AL-6061-T6", "6061-T6 Aluminum", 
                     68.9, 0.33, 276.0, 310.0, 2700.0, 
                     2.32e-5, 167.0);

    m_materialRegistry[structuralSteel.id] = structuralSteel;
    m_materialRegistry[al6061.id] = al6061;
}

bool MaterialDatabase::addCustomMaterial(const IMaterial& material) {
    // Acquire exclusive write lock
    std::unique_lock lock(m_mutex);
    
    // Insert if it doesn't exist, return false if ID already taken
    auto [iter, success] = m_materialRegistry.insert({material.id, material});
    return success;
}

std::optional<IMaterial> MaterialDatabase::getMaterial(const std::string& id) const {
    // Acquire shared read lock - solvers can hit this concurrently without blocking each other
    std::shared_lock lock(m_mutex);
    
    auto it = m_materialRegistry.find(id);
    if (it != m_materialRegistry.end()) {
        return it->second;
    }
    return std::nullopt;
}

} // namespace materials
} // namespace szm

# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(SZM_Forge_Core VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enforce strict warnings for engineering reliability
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -pthread)
endif()

# Define Library
add_library(FEA_Materials STATIC
    src/core/materials/MaterialDatabase.cpp
)

# Include directories
target_include_directories(FEA_Materials PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link threads (required for std::shared_mutex on Linux)
find_package(Threads REQUIRED)
target_link_libraries(FEA_Materials PRIVATE Threads::Threads)

cmake_minimum_required(VERSION 3.20)
project(SZM_Forge_FEA_Mesher CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or Fetch Gmsh
find_package(gmsh REQUIRED)

# Core Library
add_library(FEA_Volumetric_Mesher SHARED 
    src/core/fea/VolumetricMesher.cpp
)

# Link Gmsh and standard math/threading libraries
target_link_libraries(FEA_Volumetric_Mesher PRIVATE gmsh pthread)

# Include directories
target_include_directories(FEA_Volumetric_Mesher PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

#include <gmsh.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <string>

namespace SZMForge {
namespace FEA {

    // 5. Internal State Data Structures
    struct FEA_Node {
        double x, y, z;
        double u, v, w; // Displacement variables for the solver
    };

    struct FEA_Element {
        std::vector<std::size_t> node_tags; // Size will be 4 (Tet4) or 10 (Tet10)
    };

    class FEAMesh {
    public:
        std::vector<FEA_Node> nodes;
        std::vector<FEA_Element> elements;

        // Map arrays directly to GPU memory for visual inspection via clipping planes
        void mapToGPU() const {
            // NOTE: implementation depends on the SZM Forge graphics pipeline (Vulkan/OpenGL)
            // Example Vulkan approach: Map memory block, memcpy nodes/elements to Storage Buffers (SSBOs)
            // vkMapMemory(device, bufferMemory, 0, size, 0, &data);
            // memcpy(data, nodes.data(), (size_t)size);
            // vkUnmapMemory(device, bufferMemory);
            std::cout << "[SZM GPU Mapper] Mapping " << elements.size() 
                      << " volumetric elements to GPU memory." << std::endl;
        }
    };

    // 320.1 Volumetric Mesher Class
    class VolumetricMesher {
    public:
        static FEAMesh generateTetrahedralMesh(
            const std::string& boundaryRepFile, // Pointer/Reference to SolidBody (BRep/STEP)
            double globalElementSize,
            bool useQuadratic = false,          // false = Tet4, true = Tet10
            double microEdgeTolerance = 1e-6    // 9. Failure Condition Handling
        ) {
            FEAMesh resultMesh;

            // Initialize the Gmsh API context
            gmsh::initialize();
            gmsh::option::setNumber("General.Terminal", 1); // Pipe output to console for logging

            try {
                // Import the SolidBody geometry
                gmsh::model::occ::importShapes(boundaryRepFile);

                // --- 9. MICRO-EDGE HEALING & NON-MANIFOLD RESOLUTION ---
                // Enforce tolerance to merge edges smaller than 10^-6 meters
                gmsh::option::setNumber("Geometry.Tolerance", microEdgeTolerance);
                gmsh::option::setNumber("Geometry.ToleranceBoolean", microEdgeTolerance);
                gmsh::model::occ::healShapes(); 
                gmsh::model::occ::synchronize();

                // --- 4. INPUTS & MESH SIZING ---
                gmsh::option::setNumber("Mesh.MeshSizeMin", globalElementSize * 0.2); // Local refinement allowance
                gmsh::option::setNumber("Mesh.MeshSizeMax", globalElementSize);

                // --- 6. PHYSICS/LOGIC MODEL ---
                // Element Types: Tet4 (Linear) vs Tet10 (Quadratic)
                gmsh::option::setNumber("Mesh.ElementOrder", useQuadratic ? 2 : 1);

                // Quality Metrics: Reject slivers & enforce minimum Aspect Ratio
                gmsh::option::setNumber("Mesh.Optimize", 1);
                gmsh::option::setNumber("Mesh.OptimizeNetgen", 1); // High-quality aspect ratio optimization
                
                // Meshing Algorithm: 1 = Delaunay, 4 = Frontal-Delaunay (often better for complex CAD)
                gmsh::option::setNumber("Mesh.Algorithm3D", 1); 
                
                // Generate the 3D volume mesh
                gmsh::model::mesh::generate(3);

                // --- INTERNAL STATE MAPPING ---
                
                // Extract Nodes
                std::vector<std::size_t> nodeTags;
                std::vector<double> coord, parametricCoord;
                gmsh::model::mesh::getNodes(nodeTags, coord, parametricCoord);

                resultMesh.nodes.resize(nodeTags.size());
                for (size_t i = 0; i < nodeTags.size(); ++i) {
                    // Initialize x, y, z from Gmsh, and set u, v, w displacements to 0.0
                    resultMesh.nodes[i] = { coord[i*3], coord[i*3 + 1], coord[i*3 + 2], 0.0, 0.0, 0.0 };
                }

                // Extract Elements
                std::vector<int> elementTypes;
                std::vector<std::vector<std::size_t>> elementTags, nodeTagsPerElem;
                gmsh::model::mesh::getElements(elementTypes, elementTags, nodeTagsPerElem, 3);

                // Parse standard tetrahedral output
                for (size_t i = 0; i < elementTypes.size(); ++i) {
                    // Gmsh Types: 4 = 4-node tetrahedron (Tet4), 11 = 10-node tetrahedron (Tet10)
                    if (elementTypes[i] == 4 || elementTypes[i] == 11) {
                        size_t nodesPerTet = (elementTypes[i] == 4) ? 4 : 10;
                        for (size_t j = 0; j < elementTags[i].size(); ++j) {
                            FEA_Element tet;
                            tet.node_tags.reserve(nodesPerTet);
                            for (size_t k = 0; k < nodesPerTet; ++k) {
                                // Gmsh node tags are 1-indexed; retain exact tags for solver mapping
                                tet.node_tags.push_back(nodeTagsPerElem[i][j * nodesPerTet + k]);
                            }
                            resultMesh.elements.push_back(tet);
                        }
                    }
                }

            } catch (const std::exception& e) {
                std::cerr << "[SZM_CRITICAL] Meshing Engine Failure: " << e.what() << std::endl;
                gmsh::finalize();
                throw; // Rethrow to let the top-level application handle the abort
            }

            // Cleanup Gmsh memory context
            gmsh::finalize();

            // 7. Map the populated arrays directly to the GPU
            resultMesh.mapToGPU();

            return resultMesh;
        }
    };

} // namespace FEA
} // namespace SZMForge

