#pragma once
#include <memory>
#include <vector>
#include <stdexcept>
// #include "../SolidBody.hpp" // Assumed topology definitions

namespace SZM::Geometry {

    // Forward declarations for topological entities
    class SolidBody;
    class Face;
    class Edge;

    enum class BooleanOpType {
        Union,
        Difference,
        Intersect
    };

    class BooleanCSG {
    public:
        /**
         * @brief Executes a CSG operation between two SolidBodies.
         * @param target The primary body being modified.
         * @param tool The body acting as the cutter/adder.
         * @param opType Union, Difference, or Intersect.
         * @return A dynamically allocated, newly combined SolidBody.
         */
        static std::shared_ptr<SolidBody> Execute(
            std::shared_ptr<SolidBody> target, 
            std::shared_ptr<SolidBody> tool, 
            BooleanOpType opType
        );

    private:
        // System-wide epsilon for co-planar floating point resolution
        static constexpr double EPSILON = 1e-6;

        // Step 1: BVH Broad Phase Culling
        static void ComputeBVHIntersections(
            const std::shared_ptr<SolidBody>& target, 
            const std::shared_ptr<SolidBody>& tool,
            std::vector<std::shared_ptr<Face>>& outTargetFaces,
            std::vector<std::shared_ptr<Face>>& outToolFaces
        );

        // Step 2: Exact Mathematical Intersection & Partitioning
        static void PartitionFaces(
            std::vector<std::shared_ptr<Face>>& targetFaces,
            std::vector<std::shared_ptr<Face>>& toolFaces
        );

        // Step 3: Ray-Cast Inside/Outside Classification
        static bool IsFaceInsideBody(
            const std::shared_ptr<Face>& face, 
            const std::shared_ptr<SolidBody>& body
        );

        // Step 4: Topological Stitching
        static std::shared_ptr<SolidBody> StitchResult(
            const std::shared_ptr<SolidBody>& target,
            const std::shared_ptr<SolidBody>& tool,
            BooleanOpType opType
        );

        // Manifold Validator (Hooks into Module 110.4)
        static bool VerifyManifold(const std::shared_ptr<SolidBody>& body);
    };

} // namespace SZM::Geometry

