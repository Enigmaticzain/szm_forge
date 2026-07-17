#include "BooleanOp.hpp"
#include "AABB.hpp"
#include "BRepQueries.hpp"
#include "TopologyValidator.hpp"
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
void BooleanCSG::PartitionFaces(
    SolidBody& target,
    SolidBody& tool,
    const std::vector<uint32_t>& targetFaceIdxs,
    const std::vector<uint32_t>& toolFaceIdxs)
{
    // Fallback implementation keeps original faces. Robust face splitting by
    // intersection curves requires full topological surgery and is out of
    // scope for this phase.
    (void)target;
    (void)tool;
    (void)targetFaceIdxs;
    (void)toolFaceIdxs;
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
