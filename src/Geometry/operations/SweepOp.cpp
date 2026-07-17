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
    
    // For now, implement a stub geometry generation (e.g. bounding box or simplified tube)
    // A robust sweeping implementation requires sweeping a coordinate frame (Frenet-Serret)
    // along the path. Here we will mock the solid by adding the bottom and top faces,
    // and connecting sides using simple translation along the first and last segments.
    // Real implementation would loft each segment along the polyline.
    
    // Start face
    std::vector<Vector3> startFace;
    Vector3 startPos = path.front();
    for (const auto& v : profile.Loops[0]) {
        startFace.push_back({v.x + startPos.x, v.y + startPos.y, startPos.z});
    }
    body.AddFaceFlipped(startFace); // Bottom cap

    // End face
    std::vector<Vector3> endFace;
    Vector3 endPos = path.back();
    for (const auto& v : profile.Loops[0]) {
        endFace.push_back({v.x + endPos.x, v.y + endPos.y, endPos.z});
    }
    body.AddFace(endFace); // Top cap

    // Connect sides linearly from start to end (ignores intermediate path points for this stub)
    size_t count = profile.Loops[0].size();
    for (size_t i = 0; i < count; ++i) {
        size_t next = (i + 1) % count;
        std::vector<Vector3> sideFace = {
            startFace[i],
            endFace[i],
            endFace[next],
            startFace[next]
        };
        body.AddFace(sideFace);
    }

    body.Stitch();
    return body;
}

} // namespace SZM::Geometry::Operations
