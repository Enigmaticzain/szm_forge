#include "AABB.hpp"
#include <algorithm>
#include <numeric>

namespace SZM::Geometry {

// ================================================================
//  BVH Tree — Top-down median-split construction
// ================================================================

void BVHTree::Build(const std::vector<AABB>& faceBoxes) {
    m_Nodes.clear();
    if (faceBoxes.empty()) return;

    // Reserve rough estimate: 2N-1 nodes for N leaves
    m_Nodes.reserve(faceBoxes.size() * 2);

    // Create face index array
    std::vector<uint32_t> faceIndices(faceBoxes.size());
    std::iota(faceIndices.begin(), faceIndices.end(), 0u);

    BuildRecursive(faceIndices, faceBoxes,
                   0, static_cast<int>(faceIndices.size()));
}

int32_t BVHTree::BuildRecursive(std::vector<uint32_t>& faceIndices,
                                 const std::vector<AABB>& faceBoxes,
                                 int start, int end) {
    int32_t nodeIdx = static_cast<int32_t>(m_Nodes.size());
    m_Nodes.emplace_back();
    BVHNode& node = m_Nodes.back();

    // Compute bounds for this range
    for (int i = start; i < end; ++i) {
        node.Bounds.Expand(faceBoxes[faceIndices[static_cast<size_t>(i)]]);
    }

    int count = end - start;

    // Leaf node
    if (count == 1) {
        node.IsLeaf = true;
        node.FaceIndex = faceIndices[static_cast<size_t>(start)];
        return nodeIdx;
    }

    // Interior node: split along longest axis at median
    node.IsLeaf = false;
    int axis = node.Bounds.LongestAxis();
    int mid = start + count / 2;

    // Sort by centroid along the chosen axis
    std::nth_element(
        faceIndices.begin() + start,
        faceIndices.begin() + mid,
        faceIndices.begin() + end,
        [&faceBoxes, axis](uint32_t a, uint32_t b) {
            Vector3 ca = faceBoxes[a].Center();
            Vector3 cb = faceBoxes[b].Center();
            switch (axis) {
                case 0: return ca.x < cb.x;
                case 1: return ca.y < cb.y;
                default: return ca.z < cb.z;
            }
        }
    );

    // Build children recursively
    // Note: we must capture nodeIdx because m_Nodes may reallocate
    int32_t leftChild = BuildRecursive(faceIndices, faceBoxes, start, mid);
    int32_t rightChild = BuildRecursive(faceIndices, faceBoxes, mid, end);

    // After potential reallocation, use nodeIdx to access
    m_Nodes[static_cast<size_t>(nodeIdx)].Left = leftChild;
    m_Nodes[static_cast<size_t>(nodeIdx)].Right = rightChild;

    return nodeIdx;
}

// ================================================================
//  Overlap Queries
// ================================================================

std::vector<std::pair<uint32_t, uint32_t>>
BVHTree::FindOverlappingFaces(const BVHTree& other) const {
    std::vector<std::pair<uint32_t, uint32_t>> results;
    if (IsEmpty() || other.IsEmpty()) return results;
    QueryOverlapRecursive(0, other, 0, results);
    return results;
}

void BVHTree::QueryOverlapRecursive(
    int32_t nodeA, const BVHTree& treeB, int32_t nodeB,
    std::vector<std::pair<uint32_t, uint32_t>>& results) const
{
    if (nodeA < 0 || nodeB < 0) return;

    const BVHNode& a = m_Nodes[static_cast<size_t>(nodeA)];
    const BVHNode& b = treeB.m_Nodes[static_cast<size_t>(nodeB)];

    // Early exit if bounding boxes don't overlap
    if (!a.Bounds.Overlaps(b.Bounds)) return;

    // Both leaves: report face pair
    if (a.IsLeaf && b.IsLeaf) {
        results.emplace_back(a.FaceIndex, b.FaceIndex);
        return;
    }

    // Descend into the larger node first for balanced traversal
    if (a.IsLeaf || (!b.IsLeaf && a.Bounds.SurfaceArea() < b.Bounds.SurfaceArea())) {
        // Split B
        QueryOverlapRecursive(nodeA, treeB, b.Left, results);
        QueryOverlapRecursive(nodeA, treeB, b.Right, results);
    } else {
        // Split A
        QueryOverlapRecursive(a.Left, treeB, nodeB, results);
        QueryOverlapRecursive(a.Right, treeB, nodeB, results);
    }
}

std::vector<uint32_t> BVHTree::QueryOverlapping(const AABB& queryBox) const {
    std::vector<uint32_t> results;
    if (!IsEmpty()) {
        QueryBoxRecursive(0, queryBox, results);
    }
    return results;
}

void BVHTree::QueryBoxRecursive(int32_t nodeIdx, const AABB& queryBox,
                                 std::vector<uint32_t>& results) const {
    if (nodeIdx < 0) return;

    const BVHNode& node = m_Nodes[static_cast<size_t>(nodeIdx)];

    if (!node.Bounds.Overlaps(queryBox)) return;

    if (node.IsLeaf) {
        results.push_back(node.FaceIndex);
        return;
    }

    QueryBoxRecursive(node.Left, queryBox, results);
    QueryBoxRecursive(node.Right, queryBox, results);
}

// ================================================================
//  Ray Queries
// ================================================================

std::vector<uint32_t> BVHTree::RayQuery(const Vector3& origin,
                                         const Vector3& direction) const {
    std::vector<uint32_t> results;
    if (IsEmpty()) return results;

    // Precompute inverse direction for slab test
    Vector3 invDir{
        std::abs(direction.x) > 1e-12 ? 1.0 / direction.x : 1e12,
        std::abs(direction.y) > 1e-12 ? 1.0 / direction.y : 1e12,
        std::abs(direction.z) > 1e-12 ? 1.0 / direction.z : 1e12
    };

    RayQueryRecursive(0, origin, invDir, results);
    return results;
}

void BVHTree::RayQueryRecursive(int32_t nodeIdx, const Vector3& origin,
                                 const Vector3& invDir,
                                 std::vector<uint32_t>& results) const {
    if (nodeIdx < 0) return;

    const BVHNode& node = m_Nodes[static_cast<size_t>(nodeIdx)];

    double tMin, tMax;
    if (!node.Bounds.RayIntersects(origin, invDir, tMin, tMax)) return;

    if (node.IsLeaf) {
        results.push_back(node.FaceIndex);
        return;
    }

    RayQueryRecursive(node.Left, origin, invDir, results);
    RayQueryRecursive(node.Right, origin, invDir, results);
}

} // namespace SZM::Geometry
