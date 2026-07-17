#pragma once

#include "MathTypes.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <functional>

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  AABB — Axis-Aligned Bounding Box
    // ----------------------------------------------------------------
    struct AABB {
        Vector3 Min{std::numeric_limits<double>::max(),
                    std::numeric_limits<double>::max(),
                    std::numeric_limits<double>::max()};
        Vector3 Max{std::numeric_limits<double>::lowest(),
                    std::numeric_limits<double>::lowest(),
                    std::numeric_limits<double>::lowest()};

        /// Expand this box to contain a point
        void Expand(const Vector3& p) {
            Min.x = std::min(Min.x, p.x);
            Min.y = std::min(Min.y, p.y);
            Min.z = std::min(Min.z, p.z);
            Max.x = std::max(Max.x, p.x);
            Max.y = std::max(Max.y, p.y);
            Max.z = std::max(Max.z, p.z);
        }

        /// Expand this box to contain another box
        void Expand(const AABB& other) {
            Min.x = std::min(Min.x, other.Min.x);
            Min.y = std::min(Min.y, other.Min.y);
            Min.z = std::min(Min.z, other.Min.z);
            Max.x = std::max(Max.x, other.Max.x);
            Max.y = std::max(Max.y, other.Max.y);
            Max.z = std::max(Max.z, other.Max.z);
        }

        /// Test overlap with another AABB (with tolerance)
        [[nodiscard]] bool Overlaps(const AABB& other, double eps = 1e-6) const {
            return (Min.x <= other.Max.x + eps && Max.x >= other.Min.x - eps) &&
                   (Min.y <= other.Max.y + eps && Max.y >= other.Min.y - eps) &&
                   (Min.z <= other.Max.z + eps && Max.z >= other.Min.z - eps);
        }

        /// Surface area (used for SAH cost in BVH)
        [[nodiscard]] double SurfaceArea() const {
            Vector3 d = Max - Min;
            return 2.0 * (d.x * d.y + d.y * d.z + d.z * d.x);
        }

        /// Center of the AABB
        [[nodiscard]] Vector3 Center() const {
            return (Min + Max) * 0.5;
        }

        /// Longest axis index (0=x, 1=y, 2=z)
        [[nodiscard]] int LongestAxis() const {
            Vector3 d = Max - Min;
            if (d.x >= d.y && d.x >= d.z) return 0;
            if (d.y >= d.x && d.y >= d.z) return 1;
            return 2;
        }

        /// Ray-AABB intersection (slab method)
        /// Returns true if ray hits, with tMin/tMax range
        [[nodiscard]] bool RayIntersects(const Vector3& origin, const Vector3& invDir,
                                          double& tMin, double& tMax) const {
            double t1 = (Min.x - origin.x) * invDir.x;
            double t2 = (Max.x - origin.x) * invDir.x;
            tMin = std::min(t1, t2);
            tMax = std::max(t1, t2);

            t1 = (Min.y - origin.y) * invDir.y;
            t2 = (Max.y - origin.y) * invDir.y;
            tMin = std::max(tMin, std::min(t1, t2));
            tMax = std::min(tMax, std::max(t1, t2));

            t1 = (Min.z - origin.z) * invDir.z;
            t2 = (Max.z - origin.z) * invDir.z;
            tMin = std::max(tMin, std::min(t1, t2));
            tMax = std::min(tMax, std::max(t1, t2));

            return tMax >= std::max(tMin, 0.0);
        }

        /// Check if a point is inside
        [[nodiscard]] bool Contains(const Vector3& p) const {
            return p.x >= Min.x && p.x <= Max.x &&
                   p.y >= Min.y && p.y <= Max.y &&
                   p.z >= Min.z && p.z <= Max.z;
        }

        /// Check if the AABB is valid (min < max)
        [[nodiscard]] bool IsValid() const {
            return Min.x <= Max.x && Min.y <= Max.y && Min.z <= Max.z;
        }
    };

    // ----------------------------------------------------------------
    //  BVH Node — binary tree node for spatial partitioning
    // ----------------------------------------------------------------
    struct BVHNode {
        AABB     Bounds;
        int32_t  Left      = -1;   ///< Index of left child (-1 = leaf)
        int32_t  Right     = -1;   ///< Index of right child (-1 = leaf)
        uint32_t FaceIndex = 0;    ///< Face index (only valid for leaf nodes)
        bool     IsLeaf    = true;
    };

    // ----------------------------------------------------------------
    //  BVH Tree — Bounding Volume Hierarchy for face queries
    // ----------------------------------------------------------------
    class BVHTree {
    public:
        BVHTree() = default;

        /// Build BVH from a list of face bounding boxes
        /// @param faceBoxes  AABB for each face (indexed by face index)
        void Build(const std::vector<AABB>& faceBoxes);

        /// Find all pairs of overlapping faces between this tree and another
        [[nodiscard]] std::vector<std::pair<uint32_t, uint32_t>>
            FindOverlappingFaces(const BVHTree& other) const;

        /// Find all faces whose AABB overlaps the given box
        [[nodiscard]] std::vector<uint32_t>
            QueryOverlapping(const AABB& queryBox) const;

        /// Find all faces hit by a ray
        [[nodiscard]] std::vector<uint32_t>
            RayQuery(const Vector3& origin, const Vector3& direction) const;

        /// Get the root bounding box
        [[nodiscard]] AABB GetRootBounds() const {
            return m_Nodes.empty() ? AABB{} : m_Nodes[0].Bounds;
        }

        [[nodiscard]] bool IsEmpty() const { return m_Nodes.empty(); }

    private:
        std::vector<BVHNode> m_Nodes;

        /// Recursive build: returns node index
        int32_t BuildRecursive(std::vector<uint32_t>& faceIndices,
                               const std::vector<AABB>& faceBoxes,
                               int start, int end);

        /// Recursive overlap query between two BVH subtrees
        void QueryOverlapRecursive(int32_t nodeA, const BVHTree& treeB, int32_t nodeB,
                                   std::vector<std::pair<uint32_t, uint32_t>>& results) const;

        /// Recursive single-box overlap query
        void QueryBoxRecursive(int32_t nodeIdx, const AABB& queryBox,
                               std::vector<uint32_t>& results) const;

        /// Recursive ray query
        void RayQueryRecursive(int32_t nodeIdx, const Vector3& origin,
                               const Vector3& invDir,
                               std::vector<uint32_t>& results) const;
    };

} // namespace SZM::Geometry
