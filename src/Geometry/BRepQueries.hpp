#pragma once

#include "SolidBody.hpp"
#include "AABB.hpp"
#include <vector>
#include <optional>

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  BRepQueries — Spatial and topological queries on SolidBody
    // ----------------------------------------------------------------
    class BRepQueries {
    public:
        /// Test if a point is strictly inside a closed solid body using ray casting.
        /// Throws if the body is not closed.
        [[nodiscard]] static bool IsPointInside(const SolidBody& body, const Vector3& point);

        /// Find the closest point on the surface of the solid to a given point.
        /// Returns the closest point on the boundary.
        [[nodiscard]] static Vector3 ClosestPointOnSurface(const SolidBody& body, const Vector3& point);

        /// Compute the precise Axis-Aligned Bounding Box of the solid.
        [[nodiscard]] static AABB ComputeAABB(const SolidBody& body);

        /// Find all faces adjacent to a given face (sharing an edge).
        [[nodiscard]] static std::vector<uint32_t> GetAdjacentFaces(const SolidBody& body, uint32_t faceIdx);

        /// Get all half-edges that bound a given face.
        [[nodiscard]] static std::vector<uint32_t> GetFaceHalfEdges(const SolidBody& body, uint32_t faceIdx);

        /// Extract all isolated connected components (shells) from the solid.
        /// Returns a list of lists, where each inner list contains face indices belonging to one shell.
        [[nodiscard]] static std::vector<std::vector<uint32_t>> ExtractShells(const SolidBody& body);

        // ----------------------------------------------------------------
        //  BRep sub-element picking (ray casting)
        // ----------------------------------------------------------------

        struct RayHit {
            uint32_t index    = UINT32_MAX; ///< Face/edge/vertex index; UINT32_MAX = no hit
            double   distance = 0.0;
            Vector3  point;
        };

        [[nodiscard]] static RayHit RaycastFace(
            const SolidBody& body,
            const Vector3& rayOrigin,
            const Vector3& rayDir);

        [[nodiscard]] static RayHit RaycastEdge(
            const SolidBody& body,
            const Vector3& rayOrigin,
            const Vector3& rayDir,
            double screenRadius = 0.005);

        [[nodiscard]] static RayHit RaycastVertex(
            const SolidBody& body,
            const Vector3& rayOrigin,
            const Vector3& rayDir,
            double screenRadius = 0.005);

    private:
        /// Helper for closest point to triangle
        static Vector3 ClosestPointOnTriangle(const Vector3& p, const Vector3& a, const Vector3& b, const Vector3& c);
    };

} // namespace SZM::Geometry
