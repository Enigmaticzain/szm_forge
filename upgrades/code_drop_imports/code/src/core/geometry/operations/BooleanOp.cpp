#include "BooleanOp.hpp"
#include <iostream>

namespace SZM::Geometry {

    std::shared_ptr<SolidBody> BooleanCSG::Execute(
        std::shared_ptr<SolidBody> target, 
        std::shared_ptr<SolidBody> tool, 
        BooleanOpType opType) 
    {
        if (!target || !tool) {
            throw std::invalid_argument("[SZM-CSG Error] Invalid Target or Tool body.");
        }

        std::vector<std::shared_ptr<Face>> overlappingTargetFaces;
        std::vector<std::shared_ptr<Face>> overlappingToolFaces;

        // Step 1: Broad Phase - Use BVH/Octree to isolate faces that actually intersect
        ComputeBVHIntersections(target, tool, overlappingTargetFaces, overlappingToolFaces);

        // Optimization: If bounding boxes don't touch, handle trivial cases
        if (overlappingTargetFaces.empty() || overlappingToolFaces.empty()) {
            if (opType == BooleanOpType::Union) {
                // Return a compound body of both
            } else if (opType == BooleanOpType::Difference) {
                return target; // No cut made
            } else if (opType == BooleanOpType::Intersect) {
                return nullptr; // Empty result
            }
        }

        // Step 2: Narrow Phase - Split faces along exact geometric intersection curves
        PartitionFaces(overlappingTargetFaces, overlappingToolFaces);

        // Step 3 & 4: Classification and Stitching are handled collectively to build the final mesh
        std::shared_ptr<SolidBody> resultBody = StitchResult(target, tool, opType);

        // Final Manifold Verification (Failure Condition 11)
        if (!VerifyManifold(resultBody)) {
            throw std::runtime_error("[SZM-CSG Error] Operation resulted in non-manifold geometry.");
        }

        return resultBody;
    }

    void BooleanCSG::ComputeBVHIntersections(
        const std::shared_ptr<SolidBody>& target, 
        const std::shared_ptr<SolidBody>& tool,
        std::vector<std::shared_ptr<Face>>& outTargetFaces,
        std::vector<std::shared_ptr<Face>>& outToolFaces) 
    {
        // TODO: Traverse Target BVH against Tool BVH.
        // Populate outTargetFaces and outToolFaces with ONLY the AABB overlaps.
    }

    void BooleanCSG::PartitionFaces(
        std::vector<std::shared_ptr<Face>>& targetFaces,
        std::vector<std::shared_ptr<Face>>& toolFaces) 
    {
        // TODO: Compute Face-to-Face line/curve intersections.
        // Split target faces using the tool curves, and tool faces using target curves.
        // This generates new internal edges.
    }

    bool BooleanCSG::IsFaceInsideBody(const std::shared_ptr<Face>& face, const std::shared_ptr<SolidBody>& body) {
        // Step 3 Classification: Ray-casting algorithm
        // 1. Calculate centroid of the face.
        // 2. Shift centroid by EPSILON along the face's normal to prevent co-planar floating point ambiguity.
        // 3. Cast a ray out to infinity.
        // 4. Count intersections with 'body' faces.
        // 5. If (Intersections % 2 != 0) -> Return True (Inside).
        
        return false; // Placeholder
    }

    std::shared_ptr<SolidBody> BooleanCSG::StitchResult(
        const std::shared_ptr<SolidBody>& target,
        const std::shared_ptr<SolidBody>& tool,
        BooleanOpType opType) 
    {
        auto result = std::make_shared<SolidBody>(); // Create new empty solid

        // Evaluate Target Faces
        // target.GetFaces() would normally iterate through all topological faces
        /*
        for (auto& t_face : target->GetFaces()) {
            bool isInsideTool = IsFaceInsideBody(t_face, tool);
            
            if (opType == BooleanOpType::Union && !isInsideTool) {
                result->AddFace(t_face);
            } 
            else if (opType == BooleanOpType::Difference && !isInsideTool) {
                result->AddFace(t_face);
            }
            else if (opType == BooleanOpType::Intersect && isInsideTool) {
                result->AddFace(t_face);
            }
        }
        */

        // Evaluate Tool Faces
        /*
        for (auto& tl_face : tool->GetFaces()) {
            bool isInsideTarget = IsFaceInsideBody(tl_face, target);
            
            if (opType == BooleanOpType::Union && !isInsideTarget) {
                result->AddFace(tl_face);
            } 
            else if (opType == BooleanOpType::Difference && isInsideTarget) {
                // Requirement 9: Inherit target material for newly exposed interior faces
                tl_face->SetMaterial(target->GetMaterial());
                
                // Requirement 6: Flip normals for difference cuts
                tl_face->FlipNormal(); 
                
                result->AddFace(tl_face);
            }
            else if (opType == BooleanOpType::Intersect && isInsideTarget) {
                result->AddFace(tl_face);
            }
        }
        */

        // TODO: Rebuild winged-edge or half-edge data structure for the resulting faces
        return result;
    }

    bool BooleanCSG::VerifyManifold(const std::shared_ptr<SolidBody>& body) {
        // Module 110.4 Hook
        // Euler Characteristic Check: V - E + F = 2 (for objects with no holes)
        // Ensure every edge is shared by exactly 2 faces.
        return true; 
    }

} // namespace SZM::Geometry

# src/core/geometry/CMakeLists.txt

add_library(SZM_Geometry_Module STATIC
    operations/BooleanOp.cpp
    # SolidBody.cpp
    # BVH.cpp
)

target_include_directories(SZM_Geometry_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/operations
)

# Geometry math is heavily reliant on C++20 standard
target_compile_features(SZM_Geometry_Module PUBLIC cxx_std_20)

# If using Eigen for ray-cast matrix math (like in the constraint solver)
# target_link_libraries(SZM_Geometry_Module PUBLIC Eigen3::Eigen)

