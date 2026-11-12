#include "RevolveOp.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Geometry {

    Vector3 RevolveOp::RotateRodrigues(const Vector3& point, const Vector3& origin, const Vector3& axis, float angleRad) {
        const double len = axis.Magnitude();
        if (len < 1e-12) {
            return point;
        }
        const Vector3 unitAxis = axis * static_cast<float>(1.0 / len);
        Vector3 v = point - origin;
        float cos_a = std::cos(angleRad);
        float sin_a = std::sin(angleRad);
        
        Vector3 cross_prod = unitAxis.Cross(v);
        float dot_prod = static_cast<float>(unitAxis.Dot(v));
        
        Vector3 rotated = v * cos_a + cross_prod * sin_a + unitAxis * (dot_prod * (1.0f - cos_a));
        return rotated + origin;
    }

    void RevolveOp::ValidateProfile(const RevolveProfile& profile, const Vector3&, const Vector3& axisVector) {
        if (profile.Vertices.empty()) {
            throw std::invalid_argument("[RevolveOp] Profile cannot be empty.");
        }
        if (axisVector.Magnitude() < 1e-12) {
            throw std::invalid_argument("[RevolveOp] Axis vector cannot be zero.");
        }
    }

    SolidBody RevolveOp::Execute(
        const RevolveProfile& profile,
        const Vector3& axisOrigin,
        const Vector3& axisVector,
        float totalAngleDeg,
        int numSegments) 
    {
        ValidateProfile(profile, axisOrigin, axisVector);

        SolidBody solid;
        if (profile.Vertices.size() < 2 || numSegments < 3 || std::abs(totalAngleDeg) < 1e-6f) {
            return solid;
        }

        const Vector3 unitAxis = axisVector.Normalize();
        const bool isClosed = std::abs(std::abs(totalAngleDeg) - 360.0f) < 0.1f;
        const float effectiveDeg = isClosed ? (totalAngleDeg < 0.0f ? -360.0f : 360.0f) : totalAngleDeg;
        const float totalAngleRad = effectiveDeg * 3.14159265f / 180.0f;
        const float angleStep = totalAngleRad / numSegments;
        const int ringCount = isClosed ? numSegments : (numSegments + 1);
        
        // Generate rings of vertices (unique rings only for 360° closed loop)
        std::vector<std::vector<Vector3>> rings;
        rings.reserve(static_cast<size_t>(ringCount));
        for (int seg = 0; seg < ringCount; ++seg) {
            float angle = angleStep * seg;
            std::vector<Vector3> ring;
            ring.reserve(profile.Vertices.size());
            for (const auto& v : profile.Vertices) {
                ring.push_back(RotateRodrigues(v, axisOrigin, unitAxis, angle));
            }
            rings.push_back(std::move(ring));
        }

        // Connect adjacent rings (never wrapping an open sweep back to ring 0)
        for (int seg = 0; seg < numSegments; ++seg) {
            int ring0_idx = seg;
            int ring1_idx = isClosed ? ((seg + 1) % numSegments) : (seg + 1);
            
            const auto& ring0 = rings[ring0_idx];
            const auto& ring1 = rings[ring1_idx];
            
            for (size_t i = 0; i < ring0.size(); ++i) {
                size_t next_i = (i + 1) % ring0.size();
                
                std::vector<Vector3> quad = {
                    ring0[i],
                    ring0[next_i],
                    ring1[next_i],
                    ring1[i]
                };
                
                // Split into two triangles
                solid.AddFace({quad[0], quad[1], quad[2]});
                solid.AddFace({quad[0], quad[2], quad[3]});
            }
        }

        // Cap open revolutions when profile forms a closed polygon
        if (!isClosed && profile.Vertices.size() >= 3) {
            solid.AddFaceFlipped(rings.front());
            solid.AddFace(rings.back());
        }

        solid.Stitch();
        return solid;
    }

} // namespace SZM::Geometry
