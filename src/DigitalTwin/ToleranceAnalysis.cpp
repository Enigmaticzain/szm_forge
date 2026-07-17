#include "ToleranceAnalysis.hpp"
#include "../Geometry/TopologyValidator.hpp"
#include <iostream>
#include <random>

namespace SZM {

bool ToleranceAnalysis::ApplyManufacturingDeviation(Geometry::SolidBody& body, double toleranceBand) {
    if (toleranceBand <= 0.0) return false;

    std::cout << "[ToleranceAnalysis] Applying +/- " << toleranceBand << "m deviation to B-Rep...\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-toleranceBand, toleranceBand);

    // Naive morphing: apply random noise to all unique vertices
    for (auto& vertex : body.GetVerticesMutable()) {
        vertex.position.x += dis(gen);
        vertex.position.y += dis(gen);
        vertex.position.z += dis(gen);
    }

    // Recompute face normals and centroid
    body.CalculateCenterOfMass();
    return true;
}

bool ToleranceAnalysis::ValidateDeviatedBody(const Geometry::SolidBody& body) const {
    auto report = Geometry::TopologyValidator::Validate(body);
    return report.isManifold;
}

} // namespace SZM
