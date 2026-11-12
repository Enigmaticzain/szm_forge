#include "SweepOp.hpp"
#include <stdexcept>
#include <iostream>

namespace SZM::Geometry::Operations {

SolidBody SweepOp::SweepProfileAlongPath(const SketchProfile& profile, const std::vector<Vector3>& path) {
    if (!profile.IsValid()) {
        throw std::invalid_argument("[SweepOp] Profile must be valid.");
    }
    if (path.size() < 2) {
        throw std::invalid_argument("[SweepOp] Path must have at least 2 points.");
    }

    std::cout << "[SweepOp] Sweeping profile with " << profile.Loops[0].size() 
              << " points along path of length " << path.size() << "...\n";

    SolidBody body;

    const auto& loop = profile.Loops[0];
    const size_t count = loop.size();
    std::vector<std::vector<Vector3>> rings;
    rings.reserve(path.size());

    for (const auto& pos : path) {
        std::vector<Vector3> ring;
        ring.reserve(count);
        for (const auto& v : loop) {
            ring.push_back({v.x + pos.x, v.y + pos.y, v.z + pos.z});
        }
        rings.push_back(std::move(ring));
    }

    body.AddFaceFlipped(rings.front()); // Start cap
    body.AddFace(rings.back());         // End cap

    // Connect consecutive rings along the path (CCW outward winding)
    for (size_t p = 0; p + 1 < rings.size(); ++p) {
        for (size_t i = 0; i < count; ++i) {
            size_t next = (i + 1) % count;
            std::vector<Vector3> sideFace = {
                rings[p][i],
                rings[p][next],
                rings[p + 1][next],
                rings[p + 1][i]
            };
            body.AddFace(sideFace);
        }
    }

    body.Stitch();
    return body;
}

} // namespace SZM::Geometry::Operations
