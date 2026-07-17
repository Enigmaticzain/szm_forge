#include "MeshGenerator.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Graphics {

    uint32_t MeshGenerator::CalculateSegmentCount(double radius, double chordalError) {
        if (radius <= 0.0 || chordalError <= 0.0) {
            return 3;
        }

        // Degenerate case: error >= radius
        if (chordalError >= radius) {
            return 3;
        }

        // N = ceil(2π / arccos(1 − ε/R)) × 2
        // For R=1, ε=0.01: ratio=0.99, arccos(0.99)≈0.14107, N≈ceil(44.4)×2 = 90
        
        double ratio = 1.0 - (chordalError / radius);
        double angle = std::acos(std::max(-1.0, std::min(1.0, ratio)));
        
        if (angle <= 0.0) {
            return 3;
        }

        uint32_t segments = static_cast<uint32_t>(std::ceil(2.0 * 3.14159265359 / angle)) * 2;
        return std::max(3u, std::min(segments, 1024u));
    }

} // namespace SZM::Graphics
