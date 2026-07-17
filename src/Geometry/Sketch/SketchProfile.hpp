#pragma once

#include "../MathTypes.hpp"
#include <vector>

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  SketchProfile — 2D closed loop for solid operations
    // ----------------------------------------------------------------
    struct SketchProfile {
        /// Ordered sequence of 2D points forming one or more closed loops.
        /// (Outer loop is CCW, inner loops/holes are CW)
        std::vector<std::vector<Vector3>> Loops;
        
        /// The normal vector of the sketch plane
        Vector3 PlaneNormal{0, 0, 1};
        
        /// The origin of the sketch plane in 3D space
        Vector3 PlaneOrigin{0, 0, 0};

        /// Check if the profile has at least one valid loop
        [[nodiscard]] bool IsValid() const {
            if (Loops.empty()) return false;
            return Loops[0].size() >= 3;
        }
    };

} // namespace SZM::Geometry
