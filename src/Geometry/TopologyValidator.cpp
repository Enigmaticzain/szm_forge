#include "TopologyValidator.hpp"
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <cmath>

namespace SZM::Geometry {

// ================================================================
//  ValidationReport::ToString
// ================================================================

std::string ValidationReport::ToString() const {
    std::ostringstream ss;
    ss << "=== Topology Validation Report ===\n"
       << "  Vertices:      " << vertexCount << "\n"
       << "  Faces:         " << faceCount << "\n"
       << "  Edges:         " << edgeCount << "\n"
       << "  Euler (V-E+F): " << eulerCharacteristic << "\n"
       << "  Volume:        " << volume << " m³\n"
       << "  Surface Area:  " << surfaceArea << " m²\n"
       << "\n"
       << "  Manifold:      " << (isManifold ? "YES" : "NO") << "\n"
       << "  Watertight:    " << (isWatertight ? "YES" : "NO") << "\n"
       << "  Normals OK:    " << (hasConsistentNormals ? "YES" : "NO") << "\n"
       << "  Euler OK:      " << (passesEulerCheck ? "YES" : "NO") << "\n"
       << "\n"
       << "  Boundary Edges:      " << boundaryEdgeCount << "\n"
       << "  Degenerate Edges:    " << degenerateEdgeCount << "\n"
       << "  Non-Manifold Verts:  " << nonManifoldVertexCount << "\n"
       << "  Self-Intersections:  " << selfIntersectionCount << "\n"
       << "\n"
       << "  Overall: " << (IsValid() ? "VALID" : "INVALID") << "\n";
    return ss.str();
}

// ================================================================
//  Validate
// ================================================================

ValidationReport TopologyValidator::Validate(
    const SolidBody& body,
    double edgeTolerance,
    bool checkSelfIntersection)
{
    ValidationReport report;

    report.vertexCount = static_cast<uint32_t>(body.GetVertexCount());
    report.faceCount   = static_cast<uint32_t>(body.GetFaceCount());
    report.edgeCount   = static_cast<uint32_t>(body.GetEdgeCount());

    // Check manifold (every half-edge has a twin)
    report.isManifold = CheckManifold(body, report.boundaryEdges);
    report.boundaryEdgeCount = static_cast<uint32_t>(report.boundaryEdges.size());
    report.isWatertight = (report.boundaryEdgeCount == 0);

    // Euler-Poincaré
    report.passesEulerCheck = CheckEuler(body, report.eulerCharacteristic);

    // Normal consistency
    report.hasConsistentNormals = CheckNormalConsistency(body);

    // Degenerate edges
    FindDegenerateEdges(body, edgeTolerance, report.degenerateEdges);
    report.degenerateEdgeCount = static_cast<uint32_t>(report.degenerateEdges.size());

    // Non-manifold vertices
    FindNonManifoldVertices(body, report.nonManifoldVertices);
    report.nonManifoldVertexCount = static_cast<uint32_t>(report.nonManifoldVertices.size());

    // Surface area
    report.surfaceArea = ComputeSurfaceArea(body);

    // Volume (only if closed)
    if (body.IsClosed()) {
        try {
            report.volume = body.CalculateVolume();
        } catch (...) {
            report.volume = 0.0;
        }
    }

    // Self-intersection (expensive, optional)
    if (checkSelfIntersection) {
        FindSelfIntersections(body, report.selfIntersections);
        report.selfIntersectionCount =
            static_cast<uint32_t>(report.selfIntersections.size());
    }

    return report;
}

// ================================================================
//  CheckManifold — every half-edge must have a twin
// ================================================================

bool TopologyValidator::CheckManifold(const SolidBody& body,
                                       std::vector<uint32_t>& outBoundaryEdges) {
    outBoundaryEdges.clear();
    const auto& halfEdges = body.GetHalfEdges();

    for (uint32_t i = 0; i < static_cast<uint32_t>(halfEdges.size()); ++i) {
        if (halfEdges[i].twin == NullHandle) {
            outBoundaryEdges.push_back(i);
        }
    }
    return outBoundaryEdges.empty();
}

// ================================================================
//  CheckEuler — V - E + F = 2 (sphere) or 0 (torus)
// ================================================================

bool TopologyValidator::CheckEuler(const SolidBody& body, int64_t& outChi) {
    int64_t V = static_cast<int64_t>(body.GetVertexCount());
    int64_t E = static_cast<int64_t>(body.GetEdgeCount());
    int64_t F = static_cast<int64_t>(body.GetFaceCount());

    outChi = V - E + F;

    // Accept genus-0 (sphere-like, chi=2) and genus-1 (torus-like, chi=0)
    return (outChi == 2 || outChi == 0);
}

// ================================================================
//  CheckNormalConsistency — all normals should point outward
// ================================================================

bool TopologyValidator::CheckNormalConsistency(const SolidBody& body) {
    if (!body.IsClosed()) return false;

    // For a closed manifold with consistent outward normals,
    // the signed volume should be positive.
    try {
        // Compute signed volume directly
        double signedVolume = 0.0;
        for (uint32_t fi = 0; fi < static_cast<uint32_t>(body.GetFaceCount()); ++fi) {
            auto verts = body.GetFaceVertices(fi);
            if (verts.size() < 3) continue;

            const Vector3& v0 = verts[0];
            for (size_t i = 1; i + 1 < verts.size(); ++i) {
                signedVolume += v0.Dot(verts[i].Cross(verts[i + 1])) / 6.0;
            }
        }

        // Positive signed volume = outward normals (CCW winding)
        return signedVolume > 0.0;
    } catch (...) {
        return false;
    }
}

// ================================================================
//  FindDegenerateEdges — edges shorter than tolerance
// ================================================================

void TopologyValidator::FindDegenerateEdges(const SolidBody& body, double tolerance,
                                              std::vector<uint32_t>& outEdges) {
    outEdges.clear();
    const auto& halfEdges = body.GetHalfEdges();
    const auto& vertices  = body.GetVertices();
    double tolSq = tolerance * tolerance;

    for (uint32_t i = 0; i < static_cast<uint32_t>(halfEdges.size()); ++i) {
        // Only check each edge once (skip twins)
        if (halfEdges[i].twin != NullHandle && halfEdges[i].twin < i) continue;

        uint32_t vTo = halfEdges[i].vertex;
        uint32_t vFrom = halfEdges[halfEdges[i].prev].vertex;

        if (vTo >= vertices.size() || vFrom >= vertices.size()) continue;

        Vector3 diff = vertices[vTo].position - vertices[vFrom].position;
        if (diff.MagnitudeSq() < tolSq) {
            outEdges.push_back(i);
        }
    }
}

// ================================================================
//  FindNonManifoldVertices
// ================================================================

void TopologyValidator::FindNonManifoldVertices(const SolidBody& body,
                                                  std::vector<uint32_t>& outVertices) {
    outVertices.clear();
    const auto& halfEdges = body.GetHalfEdges();
    const auto& vertices  = body.GetVertices();

    // For each vertex, the outgoing half-edges should form a single cycle
    // via the twin-next traversal. If not, the vertex is non-manifold.
    for (uint32_t vi = 0; vi < static_cast<uint32_t>(vertices.size()); ++vi) {
        uint32_t startHe = vertices[vi].halfEdge;
        if (startHe == NullHandle) {
            outVertices.push_back(vi);
            continue;
        }

        // Count edges around this vertex
        uint32_t currHe = startHe;
        uint32_t count = 0;
        const uint32_t maxIter = static_cast<uint32_t>(halfEdges.size()) + 1;

        do {
            if (currHe >= halfEdges.size()) {
                outVertices.push_back(vi);
                goto next_vertex;
            }

            uint32_t twinHe = halfEdges[currHe].twin;
            if (twinHe == NullHandle) {
                // Boundary vertex — not necessarily non-manifold,
                // but mark for open shells
                break;
            }

            currHe = halfEdges[twinHe].next;
            ++count;

            if (count > maxIter) {
                // Infinite loop — non-manifold
                outVertices.push_back(vi);
                goto next_vertex;
            }
        } while (currHe != startHe);

        next_vertex:;
    }
}

// ================================================================
//  ComputeSurfaceArea
// ================================================================

double TopologyValidator::ComputeSurfaceArea(const SolidBody& body) {
    double totalArea = 0.0;

    for (uint32_t fi = 0; fi < static_cast<uint32_t>(body.GetFaceCount()); ++fi) {
        auto verts = body.GetFaceVertices(fi);
        if (verts.size() < 3) continue;

        const Vector3& v0 = verts[0];
        for (size_t i = 1; i + 1 < verts.size(); ++i) {
            Vector3 e1 = verts[i] - v0;
            Vector3 e2 = verts[i + 1] - v0;
            totalArea += e1.Cross(e2).Magnitude() * 0.5;
        }
    }

    return totalArea;
}

// ================================================================
//  FindSelfIntersections — BVH-accelerated
// ================================================================

void TopologyValidator::FindSelfIntersections(
    const SolidBody& body,
    std::vector<std::pair<uint32_t, uint32_t>>& outPairs)
{
    outPairs.clear();
    uint32_t faceCount = static_cast<uint32_t>(body.GetFaceCount());
    if (faceCount < 2) return;

    // Build per-face AABBs
    std::vector<AABB> faceBoxes(faceCount);
    for (uint32_t fi = 0; fi < faceCount; ++fi) {
        auto verts = body.GetFaceVertices(fi);
        for (const auto& v : verts) {
            faceBoxes[fi].Expand(v);
        }
    }

    // Build BVH
    BVHTree bvh;
    bvh.Build(faceBoxes);

    // Self-overlap: query the tree against itself
    // For self-intersection, we use a simpler O(n²) approach on BVH leaf pairs
    // and filter out adjacent faces (which naturally share edges)
    for (uint32_t fi = 0; fi < faceCount; ++fi) {
        auto candidates = bvh.QueryOverlapping(faceBoxes[fi]);
        for (uint32_t ci : candidates) {
            if (ci <= fi) continue; // Avoid duplicates

            // Skip adjacent faces (they share edges by construction)
            // Simple heuristic: faces sharing vertices are adjacent
            auto vertsA = body.GetFaceVertices(fi);
            auto vertsB = body.GetFaceVertices(ci);

            bool adjacent = false;
            for (const auto& va : vertsA) {
                for (const auto& vb : vertsB) {
                    if ((va - vb).MagnitudeSq() < 1e-12) {
                        adjacent = true;
                        break;
                    }
                }
                if (adjacent) break;
            }

            if (!adjacent) {
                // TODO: Exact triangle-triangle intersection test
                // For now, AABB overlap of non-adjacent faces is a warning
                outPairs.emplace_back(fi, ci);
            }
        }
    }
}

// ================================================================
//  RepairAll
// ================================================================

RepairReport TopologyValidator::RepairAll(SolidBody& body, double tolerance) {
    RepairReport report;

    // Step 1: Re-stitch with tolerance to merge close vertices
    body.Stitch(tolerance);

    // Step 2: Check what's left to fix
    ValidationReport validation = Validate(body, tolerance, false);

    report.healedEdges = validation.boundaryEdgeCount; // Count of remaining issues
    report.success = validation.isManifold && validation.isWatertight;

    return report;
}

} // namespace SZM::Geometry
