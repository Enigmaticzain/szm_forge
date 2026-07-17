#pragma once

#include "SolidBody.hpp"
#include "AABB.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  Validation Report — comprehensive topology analysis
    // ----------------------------------------------------------------
    struct ValidationReport {
        bool isManifold           = false;  ///< Every half-edge has a twin
        bool isWatertight         = false;  ///< No boundary edges
        bool hasConsistentNormals = false;  ///< All face normals point outward
        bool passesEulerCheck     = false;  ///< V - E + F = 2 (or 0 for torus)

        uint32_t vertexCount          = 0;
        uint32_t faceCount            = 0;
        uint32_t edgeCount            = 0;
        uint32_t boundaryEdgeCount    = 0;   ///< Half-edges without twins
        uint32_t degenerateEdgeCount  = 0;   ///< Edges shorter than tolerance
        uint32_t nonManifoldVertexCount = 0; ///< Vertices with inconsistent fan
        uint32_t selfIntersectionCount  = 0; ///< Self-intersecting face pairs

        int64_t eulerCharacteristic = 0;  ///< V - E + F
        double  volume              = 0.0;
        double  surfaceArea         = 0.0;

        std::vector<uint32_t> boundaryEdges;      ///< Indices of unpaired half-edges
        std::vector<uint32_t> degenerateEdges;     ///< Indices of micro-edges
        std::vector<uint32_t> nonManifoldVertices; ///< Vertex indices
        std::vector<std::pair<uint32_t, uint32_t>> selfIntersections; ///< Face index pairs

        /// True if the mesh is fully valid for solid operations
        [[nodiscard]] bool IsValid() const {
            return isManifold && isWatertight && hasConsistentNormals &&
                   passesEulerCheck && selfIntersectionCount == 0;
        }

        /// Generate a human-readable report string
        [[nodiscard]] std::string ToString() const;
    };

    // ----------------------------------------------------------------
    //  Repair Actions — what was fixed during repair
    // ----------------------------------------------------------------
    struct RepairReport {
        uint32_t mergedVertices       = 0;
        uint32_t healedEdges          = 0;
        uint32_t removedDegenerates   = 0;
        uint32_t flippedNormals       = 0;
        bool     success              = false;
    };

    // ----------------------------------------------------------------
    //  TopologyValidator — comprehensive B-Rep validation & repair
    // ----------------------------------------------------------------
    class TopologyValidator {
    public:
        /// Run full validation suite on a SolidBody
        /// @param body           The solid to validate
        /// @param edgeTolerance  Minimum allowed edge length [m]
        /// @param checkSelfIntersection  Enable expensive self-intersection test
        [[nodiscard]] static ValidationReport Validate(
            const SolidBody& body,
            double edgeTolerance = 1e-6,
            bool checkSelfIntersection = false
        );

        /// Attempt automatic repair of common topology issues
        /// @param body       The solid to repair (modified in place)
        /// @param tolerance  Merge tolerance for vertex welding
        /// @return Report of what was fixed
        static RepairReport RepairAll(SolidBody& body, double tolerance = 1e-6);

    private:
        /// Check that every half-edge has a twin
        static bool CheckManifold(const SolidBody& body,
                                  std::vector<uint32_t>& outBoundaryEdges);

        /// Check Euler-Poincaré characteristic
        static bool CheckEuler(const SolidBody& body, int64_t& outChi);

        /// Check normal consistency via signed-volume test
        static bool CheckNormalConsistency(const SolidBody& body);

        /// Find edges shorter than tolerance
        static void FindDegenerateEdges(const SolidBody& body, double tolerance,
                                         std::vector<uint32_t>& outEdges);

        /// Find non-manifold vertices (edge fan doesn't form a single loop)
        static void FindNonManifoldVertices(const SolidBody& body,
                                             std::vector<uint32_t>& outVertices);

        /// Compute surface area by summing triangulated face areas
        static double ComputeSurfaceArea(const SolidBody& body);

        /// Expensive self-intersection test using BVH
        static void FindSelfIntersections(
            const SolidBody& body,
            std::vector<std::pair<uint32_t, uint32_t>>& outPairs
        );
    };

} // namespace SZM::Geometry
