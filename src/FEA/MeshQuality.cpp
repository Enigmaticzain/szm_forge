#include "MeshQuality.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::FEA {

ElementQuality MeshQuality::ComputeElementQuality(const FEAElement& el, const FEAMesh& mesh) {
    ElementQuality q;
    q.aspectRatio = 1.0;
    q.jacobianRatio = 1.0;
    q.skewness = 0.0;
    q.edgeLengthRatio = 1.0;

    if (el.NodeIDs.size() < 4) return q;

    const Vec3d& p0 = mesh.Nodes[el.NodeIDs[0]].Position;
    const Vec3d& p1 = mesh.Nodes[el.NodeIDs[1]].Position;
    const Vec3d& p2 = mesh.Nodes[el.NodeIDs[2]].Position;
    const Vec3d& p3 = mesh.Nodes[el.NodeIDs[3]].Position;

    // Edge vectors
    Vec3d e01 = p1 - p0;
    Vec3d e02 = p2 - p0;
    Vec3d e03 = p3 - p0;
    Vec3d e12 = p2 - p1;
    Vec3d e13 = p3 - p1;
    Vec3d e23 = p3 - p2;

    double l01 = e01.norm();
    double l02 = e02.norm();
    double l03 = e03.norm();
    double l12 = e12.norm();
    double l13 = e13.norm();
    double l23 = e23.norm();

    double edges[] = {l01, l02, l03, l12, l13, l23};
    double minEdge = *std::min_element(std::begin(edges), std::end(edges));
    double maxEdge = *std::max_element(std::begin(edges), std::end(edges));

    if (minEdge > 1e-12) {
        q.edgeLengthRatio = maxEdge / minEdge;
    } else {
        q.edgeLengthRatio = 1000.0; // arbitrarily large
    }

    // Volume
    double vol = std::abs(e01.dot(e02.cross(e03))) / 6.0;

    // Radius of circumsphere (approximate for aspect ratio)
    // For a regular tetrahedron, ratio is 1.0. 
    // Simplified aspect ratio = max_edge / min_altitude
    // Here we just use edge length ratio as a simple proxy for aspect ratio
    q.aspectRatio = q.edgeLengthRatio;

    // Skewness proxy (0 = equilateral, 1 = degenerate)
    // Ideal volume of tet with edge = maxEdge:
    double idealVol = (std::sqrt(2.0) / 12.0) * maxEdge * maxEdge * maxEdge;
    if (idealVol > 1e-12) {
        q.skewness = std::max(0.0, 1.0 - (vol / idealVol));
    } else {
        q.skewness = 1.0;
    }

    // Jacobian determinant is related to volume
    q.jacobianRatio = 1.0; // Requires full integration points for Tet10 to be meaningful

    return q;
}

double MeshQuality::ComputeMeshQualityScore(const FEAMesh& mesh) {
    if (mesh.Elements.empty()) return 0.0;

    double totalScore = 0.0;
    for (const auto& el : mesh.Elements) {
        ElementQuality q = ComputeElementQuality(el, mesh);
        // Simple heuristic: skewness is [0..1] where 0 is perfect.
        // We want score [0..1] where 1 is perfect.
        double score = 1.0 - q.skewness;
        totalScore += score;
    }
    return totalScore / mesh.Elements.size();
}

std::vector<uint32_t> MeshQuality::FindBadElements(const FEAMesh& mesh, double threshold) {
    std::vector<uint32_t> badElements;
    for (size_t i = 0; i < mesh.Elements.size(); ++i) {
        ElementQuality q = ComputeElementQuality(mesh.Elements[i], mesh);
        double score = 1.0 - q.skewness;
        if (score < threshold) {
            badElements.push_back(static_cast<uint32_t>(i));
        }
    }
    return badElements;
}

} // namespace SZM::FEA
