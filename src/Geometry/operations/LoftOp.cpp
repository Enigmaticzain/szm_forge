#include "LoftOp.hpp"
#include <stdexcept>
#include <iostream>

namespace SZM::Geometry::Operations {

SolidBody LoftOp::LoftProfiles(const std::vector<SketchProfile>& profiles) {
    if (profiles.size() < 2) {
        throw std::invalid_argument("[LoftOp] Lofting requires at least 2 profiles.");
    }

    if (!profiles.front().IsValid()) {
        throw std::invalid_argument("[LoftOp] All profiles must be valid.");
    }
    size_t vertCount = profiles.front().Loops[0].size();
    for (const auto& profile : profiles) {
        if (!profile.IsValid()) {
            throw std::invalid_argument("[LoftOp] All profiles must be valid.");
        }
        if (profile.Loops[0].size() != vertCount) {
            throw std::invalid_argument("[LoftOp] All profiles must have the same number of vertices for simple lofting.");
        }
    }

    std::cout << "[LoftOp] Lofting " << profiles.size() << " profiles...\n";

    SolidBody body;

    // Preserve explicit 3D Z coordinates if profiles lie at distinct heights;
    // otherwise space coplanar 2D profiles sequentially along Z.
    bool hasDistinctZ = false;
    const double baseZ = profiles.front().Loops[0].front().z;
    for (const auto& profile : profiles) {
        for (const auto& v : profile.Loops[0]) {
            if (std::abs(v.z - baseZ) > 1e-9) {
                hasDistinctZ = true;
                break;
            }
        }
        if (hasDistinctZ) break;
    }

    auto profileZ = [&](size_t idx, const Vector3& v) -> double {
        return hasDistinctZ ? v.z : static_cast<double>(idx);
    };

    // Bottom cap (reversed winding to face outward)
    std::vector<Vector3> bottomFace;
    bottomFace.reserve(vertCount);
    for (const auto& v : profiles.front().Loops[0]) {
        bottomFace.push_back({v.x, v.y, profileZ(0, v)});
    }
    body.AddFaceFlipped(bottomFace);

    // Top cap
    std::vector<Vector3> topFace;
    topFace.reserve(vertCount);
    const size_t lastIdx = profiles.size() - 1;
    for (const auto& v : profiles.back().Loops[0]) {
        topFace.push_back({v.x, v.y, profileZ(lastIdx, v)});
    }
    body.AddFace(topFace);

    // Side faces lofting between sequential profiles (CCW outward winding)
    for (size_t p = 0; p < profiles.size() - 1; ++p) {
        for (size_t i = 0; i < vertCount; ++i) {
            size_t next = (i + 1) % vertCount;
            const auto& v00 = profiles[p].Loops[0][i];
            const auto& v01 = profiles[p].Loops[0][next];
            const auto& v10 = profiles[p + 1].Loops[0][i];
            const auto& v11 = profiles[p + 1].Loops[0][next];

            std::vector<Vector3> sideFace = {
                {v00.x, v00.y, profileZ(p, v00)},
                {v01.x, v01.y, profileZ(p, v01)},
                {v11.x, v11.y, profileZ(p + 1, v11)},
                {v10.x, v10.y, profileZ(p + 1, v10)}
            };
            body.AddFace(sideFace);
        }
    }

    body.Stitch();
    return body;
}

} // namespace SZM::Geometry::Operations
