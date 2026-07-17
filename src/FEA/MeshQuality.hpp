#pragma once

#include "FEADataTypes.hpp"
#include <vector>
#include <cstdint>

namespace SZM::FEA {

    struct ElementQuality {
        double aspectRatio;      // Ideal = 1.0, bad > 10
        double jacobianRatio;    // Min/max Jacobian determinant
        double skewness;         // Deviation from equilateral
        double edgeLengthRatio;  // Max/min edge length
    };

    class MeshQuality {
    public:
        /// Compute quality metrics for a single tetrahedron
        static ElementQuality ComputeElementQuality(const FEAElement& el, const FEAMesh& mesh);
        
        /// Compute overall mesh quality score (0.0 to 1.0, higher is better)
        static double ComputeMeshQualityScore(const FEAMesh& mesh);
        
        /// Find indices of elements that fall below a given quality threshold
        static std::vector<uint32_t> FindBadElements(const FEAMesh& mesh, double threshold = 0.3);
    };

} // namespace SZM::FEA
