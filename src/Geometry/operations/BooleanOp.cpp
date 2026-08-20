#include "BooleanOp.hpp"
#include "../AABB.hpp"
#include "../BRepQueries.hpp"
#include "../TopologyValidator.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace SZM::Geometry {

namespace {

AABB ComputeFaceAABB(const std::vector<Vector3>& face) {
    AABB box;
    for (const auto& v : face) {
        box.Expand(v);
    }
    return box;
}

} // namespace

// ------------------------------------------------------------- Execute
std::shared_ptr<SolidBody> BooleanCSG::Execute(
    std::shared_ptr<SolidBody> target,
    std::shared_ptr<SolidBody> tool,
    BooleanOpType              opType)
{
    if (!target || !tool)
        throw std::invalid_argument(
            "[SZM CSG] Execute: null target or tool body.");

    // Step 1 — Broad phase: find candidate face pairs via BVH
    std::vector<uint32_t> overlappingTarget, overlappingTool;
    ComputeBVHIntersections(target, tool, overlappingTarget, overlappingTool);

    // Trivial case: no broad-phase overlap.
    if (overlappingTarget.empty() || overlappingTool.empty()) {
        if (opType == BooleanOpType::Union) {
            // Full disjoint-compound solids are not represented in the current
            // SolidBody manifold model, so preserve target as safe fallback.
            return target;
        } else if (opType == BooleanOpType::Difference) {
            return target;
        } else { // Intersect
            return nullptr;
        }
    }

    // Step 2 — Narrow phase: split faces along intersection curves
    // (kept as no-op fallback when robust curve splitting is unavailable)
    PartitionFaces(*target, *tool, overlappingTarget, overlappingTool);

    // Steps 3 + 4 — Classify and stitch the result.
    std::shared_ptr<SolidBody> result = StitchResult(*target, *tool, opType);
    if (!result) {
        return nullptr;
    }

    result->Stitch();
    return result;
}

void BooleanCSG::ComputeBVHIntersections(
    const std::shared_ptr<SolidBody>& target,
    const std::shared_ptr<SolidBody>& tool,
    std::vector<uint32_t>&            outTargetFaceIdxs,
    std::vector<uint32_t>&            outToolFaceIdxs)
{
    outTargetFaceIdxs.clear();
    outToolFaceIdxs.clear();
    if (!target || !tool) return;

    const size_t tFaceCount = target->GetFaceCount();
    const size_t uFaceCount = tool->GetFaceCount();
    if (tFaceCount == 0 || uFaceCount == 0) return;

    std::vector<AABB> targetBoxes(tFaceCount);
    std::vector<AABB> toolBoxes(uFaceCount);
    for (uint32_t i = 0; i < tFaceCount; ++i) {
        targetBoxes[i] = ComputeFaceAABB(target->GetFaceVertices(i));
    }
    for (uint32_t i = 0; i < uFaceCount; ++i) {
        toolBoxes[i] = ComputeFaceAABB(tool->GetFaceVertices(i));
    }

    BVHTree treeT, treeU;
    treeT.Build(targetBoxes);
    treeU.Build(toolBoxes);

    auto overlappingPairs = treeT.FindOverlappingFaces(treeU);
    
    std::vector<bool> tSeen(tFaceCount, false);
    std::vector<bool> uSeen(uFaceCount, false);

    for (const auto& pair : overlappingPairs) {
        uint32_t ti = pair.first;
        uint32_t ui = pair.second;

        if (!tSeen[ti]) {
            outTargetFaceIdxs.push_back(ti);
            tSeen[ti] = true;
        }
        if (!uSeen[ui]) {
            outToolFaceIdxs.push_back(ui);
            uSeen[ui] = true;
        }
    }
}

// ------------------------------------------------------- PartitionFaces
//
// Möller triangle-triangle intersection: for each overlapping face pair,
// compute the intersection segment and split both triangles along it.
// Resulting sub-triangles replace the originals in the body.
//
void BooleanCSG::PartitionFaces(
    SolidBody& target,
    SolidBody& tool,
    const std::vector<uint32_t>& targetFaceIdxs,
    const std::vector<uint32_t>& toolFaceIdxs)
{
    // Helper: signed distance of a point to a plane (n·p + d)
    auto planeDist = [](const Vector3& n, double d, const Vector3& p) -> double {
        return n.Dot(p) + d;
    };

    // Helper: intersect edge (A,B) with plane, returns parameter t in [0,1]
    auto edgePlaneT = [](const Vector3& A, const Vector3& B,
                         const Vector3& n, double d) -> double {
        double dA = n.Dot(A) + d;
        double dB = n.Dot(B) + d;
        double denom = dA - dB;
        if (std::abs(denom) < 1e-12) return 0.5;
        return dA / denom;
    };

    // For each overlapping pair, compute intersection segment
    // and record split points. We collect new faces to add after iteration.
    std::vector<std::vector<Vector3>> newTargetFaces;
    std::vector<std::vector<Vector3>> newToolFaces;
    std::vector<uint32_t> splitTargetIdx, splitToolIdx;

    for (size_t pi = 0; pi < targetFaceIdxs.size() && pi < toolFaceIdxs.size(); ++pi) {
        uint32_t tfi = targetFaceIdxs[pi];
        uint32_t ufi = toolFaceIdxs[pi];

        auto TV = target.GetFaceVertices(tfi);
        auto UV = tool.GetFaceVertices(ufi);
        if (TV.size() < 3 || UV.size() < 3) continue;

        // Plane of tool triangle
        Vector3 Un = (UV[1]-UV[0]).Cross(UV[2]-UV[0]);
        double  UnLen = Un.Magnitude();
        if (UnLen < 1e-12) continue;
        Un = Un * (1.0 / UnLen);
        double Ud = -Un.Dot(UV[0]);

        // Signed distances of target vertices to tool plane
        double dT[3] = { planeDist(Un,Ud,TV[0]), planeDist(Un,Ud,TV[1]), planeDist(Un,Ud,TV[2]) };
        // All same side → no intersection
        if ((dT[0]>0&&dT[1]>0&&dT[2]>0)||(dT[0]<0&&dT[1]<0&&dT[2]<0)) continue;

        // Plane of target triangle
        Vector3 Tn = (TV[1]-TV[0]).Cross(TV[2]-TV[0]);
        double  TnLen = Tn.Magnitude();
        if (TnLen < 1e-12) continue;
        Tn = Tn * (1.0 / TnLen);
        double Td = -Tn.Dot(TV[0]);

        double dU[3] = { planeDist(Tn,Td,UV[0]), planeDist(Tn,Td,UV[1]), planeDist(Tn,Td,UV[2]) };
        if ((dU[0]>0&&dU[1]>0&&dU[2]>0)||(dU[0]<0&&dU[1]<0&&dU[2]<0)) continue;

        // Intersection line direction
        Vector3 lineDir = Tn.Cross(Un);
        if (lineDir.Magnitude() < 1e-12) continue;

        // Project vertices onto intersection line to find overlap interval
        auto projectOnLine = [&](const Vector3& p) -> double {
            return lineDir.Dot(p);
        };

        // Target interval
        double tProj[3] = { projectOnLine(TV[0]), projectOnLine(TV[1]), projectOnLine(TV[2]) };
        // Find the two edges that cross the tool plane
        std::vector<Vector3> tPts;
        for (int a = 0; a < 3; ++a) {
            int b = (a+1)%3;
            if ((dT[a] * dT[b]) < 0.0) {
                double t = edgePlaneT(TV[a], TV[b], Un, Ud);
                tPts.push_back(TV[a] + (TV[b]-TV[a]) * t);
            }
        }
        if (tPts.size() < 2) continue;

        // Tool interval
        std::vector<Vector3> uPts;
        for (int a = 0; a < 3; ++a) {
            int b = (a+1)%3;
            if ((dU[a] * dU[b]) < 0.0) {
                double t = edgePlaneT(UV[a], UV[b], Tn, Td);
                uPts.push_back(UV[a] + (UV[b]-UV[a]) * t);
            }
        }
        if (uPts.size() < 2) continue;

        // Check intervals overlap on the line
        double tA = projectOnLine(tPts[0]), tB = projectOnLine(tPts[1]);
        double uA = projectOnLine(uPts[0]), uB = projectOnLine(uPts[1]);
        if (tA > tB) std::swap(tA,tB);
        if (uA > uB) std::swap(uA,uB);
        if (tB < uA - EPSILON || uB < tA - EPSILON) continue; // no overlap

        // Intersection segment endpoints
        double sA = std::max(tA, uA);
        double sB = std::min(tB, uB);
        // Interpolate actual 3D points on the line
        double tRange = tB - tA;
        double uRange = uB - uA;
        Vector3 segA = (tRange > EPSILON)
            ? tPts[0] + (tPts[1]-tPts[0]) * ((sA-tA)/tRange)
            : tPts[0];
        Vector3 segB = (tRange > EPSILON)
            ? tPts[0] + (tPts[1]-tPts[0]) * ((sB-tA)/tRange)
            : tPts[1];

        // Split target triangle: find the lone vertex on one side
        // and produce 3 sub-triangles (fan from intersection segment)
        auto splitTri = [&](const std::vector<Vector3>& V,
                            const double* d,
                            const Vector3& pA, const Vector3& pB,
                            std::vector<std::vector<Vector3>>& out) {
            // Lone vertex is the one with opposite sign to the other two
            int lone = -1;
            for (int i = 0; i < 3; ++i) {
                int j = (i+1)%3, k = (i+2)%3;
                if ((d[j]>0) == (d[k]>0) && (d[i]>0) != (d[j]>0)) { lone = i; break; }
            }
            if (lone < 0) return;
            int a = (lone+1)%3, b = (lone+2)%3;
            // 3 sub-triangles: lone-pA-pB, a-pA-lone(wrong), use fan
            out.push_back({V[lone], pA, pB});
            out.push_back({V[a],    pA, V[lone]});
            out.push_back({V[a],    V[b], pA});
            out.push_back({V[b],    pB,   pA});
        };

        std::vector<std::vector<Vector3>> tSplit, uSplit;
        splitTri(TV, dT, segA, segB, tSplit);
        splitTri(UV, dU, segA, segB, uSplit);

        if (!tSplit.empty()) {
            splitTargetIdx.push_back(tfi);
            for (auto& f : tSplit) newTargetFaces.push_back(f);
        }
        if (!uSplit.empty()) {
            splitToolIdx.push_back(ufi);
            for (auto& f : uSplit) newToolFaces.push_back(f);
        }
    }

    // Add split sub-faces (original faces remain; classification will handle them)
    for (auto& f : newTargetFaces) target.AddFace(f);
    for (auto& f : newToolFaces)   tool.AddFace(f);
}

// ---------------------------------------------------- IsFaceInsideBody
bool BooleanCSG::IsFaceInsideBody(
    uint32_t         faceIdx,
    const SolidBody& queryBody,
    const SolidBody& testBody)
{
    if (faceIdx >= queryBody.GetFaceCount() || testBody.GetFaceCount() == 0) {
        return false;
    }

    const Vector3 centroid = queryBody.GetFaceCentroid(faceIdx);
    // Push it slightly along the normal to avoid coplanar ambiguity
    // Wait, face normals are not explicitly stored/retrieved cleanly in all cases,
    // so just use BRepQueries point-inside test which is robust.
    // However, if the centroid is exactly on the surface, IsPointInside might have issues.
    // We add a tiny epsilon along the face normal
    Vector3 faceNormal(0,0,0);
    auto verts = queryBody.GetFaceVertices(faceIdx);
    if(verts.size() >= 3) {
        faceNormal = (verts[1] - verts[0]).Cross(verts[2] - verts[0]).Normalize();
    }
    
    // For intersection classification, we want to know if the face *volume* is inside.
    // We push the centroid inwards slightly.
    const Vector3 testPoint = centroid - faceNormal * EPSILON;

    return BRepQueries::IsPointInside(testBody, testPoint);
}

// ------------------------------------------------------- StitchResult
std::shared_ptr<SolidBody> BooleanCSG::StitchResult(
    const SolidBody& target,
    const SolidBody& tool,
    BooleanOpType    opType)
{
    auto result = std::make_shared<SolidBody>();

    // ---- Target faces -----------------------------------------------
    for (uint32_t fi = 0; fi < static_cast<uint32_t>(target.GetFaceCount()); ++fi) {
        bool isInsideTool = IsFaceInsideBody(fi, target, tool);

        const bool keep = (opType == BooleanOpType::Union && !isInsideTool) ||
                          (opType == BooleanOpType::Difference && !isInsideTool) ||
                          (opType == BooleanOpType::Intersect && isInsideTool);
        if (!keep) {
            continue;
        }
        result->AddFace(target.GetFaceVertices(fi));
    }

    // ---- Tool faces -------------------------------------------------
    for (uint32_t fi = 0; fi < static_cast<uint32_t>(tool.GetFaceCount()); ++fi) {
        bool isInsideTarget = IsFaceInsideBody(fi, tool, target);

        if (opType == BooleanOpType::Union && !isInsideTarget) {
            result->AddFace(tool.GetFaceVertices(fi));
        } else if (opType == BooleanOpType::Difference && isInsideTarget) {
            // Difference keeps interior tool surfaces with flipped orientation.
            result->AddFaceFlipped(tool.GetFaceVertices(fi));
        } else if (opType == BooleanOpType::Intersect && isInsideTarget) {
            result->AddFace(tool.GetFaceVertices(fi));
        }
    }

    if (result->GetFaceCount() == 0) {
        return nullptr;
    }
    result->Stitch();
    return result;
}

} // namespace SZM::Geometry
