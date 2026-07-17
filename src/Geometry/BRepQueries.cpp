#include "BRepQueries.hpp"
#include <limits>
#include <cmath>
#include <queue>
#include <unordered_set>

namespace SZM::Geometry {

namespace {

bool RayIntersectsTriangle(
    const Vector3& origin,
    const Vector3& dir,
    const Vector3& v0,
    const Vector3& v1,
    const Vector3& v2,
    double& outT)
{
    const Vector3 e1 = v1 - v0;
    const Vector3 e2 = v2 - v0;
    const Vector3 p = dir.Cross(e2);
    const double det = e1.Dot(p);
    
    // EPSILON for determinant
    if (std::abs(det) < 1e-9) return false;

    const double invDet = 1.0 / det;
    const Vector3 tVec = origin - v0;
    const double u = tVec.Dot(p) * invDet;
    
    if (u < 0.0 || u > 1.0) return false;

    const Vector3 q = tVec.Cross(e1);
    const double v = dir.Dot(q) * invDet;
    
    if (v < 0.0 || u + v > 1.0) return false;

    const double t = e2.Dot(q) * invDet;
    if (t <= 1e-9) return false;

    outT = t;
    return true;
}

} // namespace

// ================================================================
//  IsPointInside (Ray Casting)
// ================================================================

bool BRepQueries::IsPointInside(const SolidBody& body, const Vector3& point) {
    if (!body.IsClosed()) {
        throw std::runtime_error("Cannot perform inside/outside test on an open shell.");
    }

    // Cast a ray in a random (but consistent) direction
    Vector3 dir(1.0, 0.4123, 0.7182);
    dir = dir.Normalize();

    int intersectionCount = 0;
    
    // Test against all faces (triangulated)
    for (uint32_t fi = 0; fi < static_cast<uint32_t>(body.GetFaceCount()); ++fi) {
        auto verts = body.GetFaceVertices(fi);
        if (verts.size() < 3) continue;

        const Vector3& v0 = verts[0];
        for (size_t i = 1; i + 1 < verts.size(); ++i) {
            double t = 0;
            if (RayIntersectsTriangle(point, dir, v0, verts[i], verts[i + 1], t)) {
                intersectionCount++;
            }
        }
    }

    // Odd number of intersections means inside
    return (intersectionCount % 2) != 0;
}

// ================================================================
//  ClosestPointOnSurface
// ================================================================

Vector3 BRepQueries::ClosestPointOnSurface(const SolidBody& body, const Vector3& point) {
    Vector3 closest(0,0,0);
    double minDistSq = std::numeric_limits<double>::max();

    for (uint32_t fi = 0; fi < static_cast<uint32_t>(body.GetFaceCount()); ++fi) {
        auto verts = body.GetFaceVertices(fi);
        if (verts.size() < 3) continue;

        const Vector3& v0 = verts[0];
        for (size_t i = 1; i + 1 < verts.size(); ++i) {
            Vector3 p = ClosestPointOnTriangle(point, v0, verts[i], verts[i+1]);
            double distSq = (p - point).MagnitudeSq();
            if (distSq < minDistSq) {
                minDistSq = distSq;
                closest = p;
            }
        }
    }
    return closest;
}

Vector3 BRepQueries::ClosestPointOnTriangle(const Vector3& p, const Vector3& a, const Vector3& b, const Vector3& c) {
    Vector3 ab = b - a;
    Vector3 ac = c - a;
    Vector3 ap = p - a;

    double d1 = ab.Dot(ap);
    double d2 = ac.Dot(ap);
    if (d1 <= 0.0 && d2 <= 0.0) return a; 

    Vector3 bp = p - b;
    double d3 = ab.Dot(bp);
    double d4 = ac.Dot(bp);
    if (d3 >= 0.0 && d4 <= d3) return b; 

    double vc = d1*d4 - d3*d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
        double v = d1 / (d1 - d3);
        return a + ab * v; 
    }

    Vector3 cp = p - c;
    double d5 = ab.Dot(cp);
    double d6 = ac.Dot(cp);
    if (d6 >= 0.0 && d5 <= d6) return c; 

    double vb = d5*d2 - d1*d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
        double w = d2 / (d2 - d6);
        return a + ac * w; 
    }

    double va = d3*d6 - d5*d4;
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0) {
        double w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + (c - b) * w; 
    }

    double denom = 1.0 / (va + vb + vc);
    double v = vb * denom;
    double w = vc * denom;
    return a + ab * v + ac * w; 
}

// ================================================================
//  ComputeAABB
// ================================================================

AABB BRepQueries::ComputeAABB(const SolidBody& body) {
    AABB box;
    for (const auto& v : body.GetVertices()) {
        box.Expand(v.position);
    }
    return box;
}

// ================================================================
//  GetAdjacentFaces
// ================================================================

std::vector<uint32_t> BRepQueries::GetAdjacentFaces(const SolidBody& body, uint32_t faceIdx) {
    std::vector<uint32_t> adjacent;
    if (faceIdx >= body.GetFaceCount()) return adjacent;

    const auto& halfEdges = body.GetHalfEdges();
    
    // Get all half-edges for this face
    auto hes = GetFaceHalfEdges(body, faceIdx);
    
    for (uint32_t heIdx : hes) {
        uint32_t twinIdx = halfEdges[heIdx].twin;
        if (twinIdx != NullHandle) {
            uint32_t adjFace = halfEdges[twinIdx].face;
            if (adjFace != faceIdx) { // Should always be true
                // Avoid duplicates if multiple edges shared (rare but possible)
                if (std::find(adjacent.begin(), adjacent.end(), adjFace) == adjacent.end()) {
                    adjacent.push_back(adjFace);
                }
            }
        }
    }
    return adjacent;
}

// ================================================================
//  GetFaceHalfEdges
// ================================================================

std::vector<uint32_t> BRepQueries::GetFaceHalfEdges(const SolidBody& body, uint32_t faceIdx) {
    std::vector<uint32_t> hes;
    if (faceIdx >= body.GetFaceCount()) return hes;

    const auto& faces = body.GetFaces();
    const auto& halfEdges = body.GetHalfEdges();

    uint32_t startHe = faces[faceIdx].halfEdge;
    if (startHe == NullHandle || startHe >= halfEdges.size()) return hes;

    uint32_t currHe = startHe;
    size_t guard = 0;
    const size_t guardMax = halfEdges.size() + 1;
    
    do {
        if (currHe >= halfEdges.size()) break;
        hes.push_back(currHe);
        currHe = halfEdges[currHe].next;
        ++guard;
    } while (currHe != startHe && currHe != NullHandle && guard < guardMax);

    return hes;
}

// ================================================================
//  ExtractShells
// ================================================================

std::vector<std::vector<uint32_t>> BRepQueries::ExtractShells(const SolidBody& body) {
    std::vector<std::vector<uint32_t>> shells;
    uint32_t faceCount = static_cast<uint32_t>(body.GetFaceCount());
    if (faceCount == 0) return shells;

    std::vector<bool> visited(faceCount, false);

    for (uint32_t i = 0; i < faceCount; ++i) {
        if (visited[i]) continue;

        std::vector<uint32_t> currentShell;
        std::queue<uint32_t> queue;
        
        queue.push(i);
        visited[i] = true;

        while (!queue.empty()) {
            uint32_t curr = queue.front();
            queue.pop();
            currentShell.push_back(curr);

            auto adjacent = GetAdjacentFaces(body, curr);
            for (uint32_t adj : adjacent) {
                if (!visited[adj]) {
                    visited[adj] = true;
                    queue.push(adj);
                }
            }
        }

        shells.push_back(std::move(currentShell));
    }

    return shells;
}

} // namespace SZM::Geometry
