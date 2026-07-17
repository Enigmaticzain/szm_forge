#pragma once

#include "../SolidBody.hpp"
#include <memory>
#include <stdexcept>

namespace SZM::Geometry {

    enum class BooleanOpType {
        Union,
        Difference,
        Intersect
    };

    /**
     * @class BooleanCSG
     * @brief Constructive Solid Geometry — Union, Difference, Intersect.
     *
     * Pipeline:
     *   1. BVH broad-phase culling  (isolate intersecting face pairs)
     *   2. Narrow-phase face partitioning  (split faces along intersection curves)
     *   3. Ray-cast inside/outside classification
     *   4. Topological stitching into a new SolidBody
     *   5. Manifold verification (Euler–Poincaré check via SolidBody::VerifyManifold)
     *
     * Status: Steps 1-4 are scaffolded with documented TODOs.
     *         Step 5 is wired and active.
     *         Full BVH + exact intersection pending 110-Series geometry engineers.
     */
    class BooleanCSG {
    public:
        /**
         * @brief Execute a CSG operation between two SolidBodies.
         * @param target  Primary body (the one being modified)
         * @param tool    Tool body (the cutter / adder)
         * @param opType  Union | Difference | Intersect
         * @return New SolidBody containing the result
         * @throws std::invalid_argument  if either body is null
         * @throws std::runtime_error     if the result is non-manifold
         */
        [[nodiscard]] static std::shared_ptr<SolidBody> Execute(
            std::shared_ptr<SolidBody> target,
            std::shared_ptr<SolidBody> tool,
            BooleanOpType              opType
        );

    private:
        static constexpr double EPSILON = 1e-6;

        // Step 1 — BVH broad phase
        static void ComputeBVHIntersections(
            const std::shared_ptr<SolidBody>& target,
            const std::shared_ptr<SolidBody>& tool,
            std::vector<uint32_t>&            outTargetFaceIdxs,
            std::vector<uint32_t>&            outToolFaceIdxs
        );

        // Step 2 — Exact face partitioning
        static void PartitionFaces(
            SolidBody& target,
            SolidBody& tool,
            const std::vector<uint32_t>& targetFaceIdxs,
            const std::vector<uint32_t>& toolFaceIdxs
        );

        // Step 3 — Ray-cast classification
        [[nodiscard]] static bool IsFaceInsideBody(
            uint32_t           faceIdx,
            const SolidBody&   queryBody,
            const SolidBody&   testBody
        );

        // Step 4 — Topological stitching
        [[nodiscard]] static std::shared_ptr<SolidBody> StitchResult(
            const SolidBody& target,
            const SolidBody& tool,
            BooleanOpType    opType
        );
    };

} // namespace SZM::Geometry
