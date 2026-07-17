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

    // Bottom cap (reversed winding to face outward)
    std::vector<Vector3> bottomFace;
    for (const auto& v : profiles.front().Loops[0]) {
        bottomFace.push_back({v.x, v.y, 0.0}); // Assuming base is at Z=0, or use a plane offset
    }
    body.AddFaceFlipped(bottomFace);

    // Top cap
    std::vector<Vector3> topFace;
    double topZ = static_cast<double>(profiles.size() - 1); // Mock Z height
    for (const auto& v : profiles.back().Loops[0]) {
        topFace.push_back({v.x, v.y, topZ});
    }
    body.AddFace(topFace);

    // Side faces lofting between sequential profiles
    for (size_t p = 0; p < profiles.size() - 1; ++p) {
        double z0 = static_cast<double>(p);
        double z1 = static_cast<double>(p + 1);

        for (size_t i = 0; i < vertCount; ++i) {
            size_t next = (i + 1) % vertCount;

            std::vector<Vector3> sideFace = {
                {profiles[p].Loops[0][i].x,       profiles[p].Loops[0][i].y,       z0},
                {profiles[p+1].Loops[0][i].x,     profiles[p+1].Loops[0][i].y,     z1},
                {profiles[p+1].Loops[0][next].x,  profiles[p+1].Loops[0][next].y,  z1},
                {profiles[p].Loops[0][next].x,    profiles[p].Loops[0][next].y,    z0}
            };
            body.AddFace(sideFace);
        }
    }

    body.Stitch();
    return body;
}

} // namespace SZM::Geometry::Operations
