#include "RevolveOp.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Geometry {

    Vector3 RevolveOp::RotateRodrigues(const Vector3& point, const Vector3& origin, const Vector3& axis, float angleRad) {
        Vector3 v = point - origin;
        float cos_a = std::cos(angleRad);
        float sin_a = std::sin(angleRad);
        
        Vector3 cross_prod = axis.Cross(v);
        float dot_prod = axis.Dot(v);
        
        Vector3 rotated = v * cos_a + cross_prod * sin_a + axis * (dot_prod * (1.0f - cos_a));
        return rotated + origin;
    }

    void RevolveOp::ValidateProfile(const RevolveProfile&, const Vector3&, const Vector3&) {
        // Stub validation
    }

    SolidBody RevolveOp::Execute(
        const RevolveProfile& profile,
        const Vector3& axisOrigin,
        const Vector3& axisVector,
        float totalAngleDeg,
        int numSegments) 
    {
        SolidBody solid;
        
        if (profile.Vertices.size() < 2 || numSegments < 3) {
            return solid;
        }

        float totalAngleRad = totalAngleDeg * 3.14159265f / 180.0f;
        float angleStep = totalAngleRad / numSegments;
        
        // Generate rings of vertices
        std::vector<std::vector<Vector3>> rings;
        for (int seg = 0; seg <= numSegments; ++seg) {
            float angle = angleStep * seg;
            std::vector<Vector3> ring;
            for (const auto& v : profile.Vertices) {
                Vector3 rotated = RotateRodrigues(v, axisOrigin, axisVector, angle);
                ring.push_back(rotated);
            }
            rings.push_back(ring);
        }

        // For 360° revolution, the last ring should match the first (weld them)
        bool isClosed = std::abs(totalAngleDeg - 360.0f) < 0.1f;
        int numRingsToConnect = isClosed ? numSegments : numSegments + 1;

        // Create side faces connecting rings
        for (int seg = 0; seg < numRingsToConnect; ++seg) {
            int ring0_idx = seg;
            int ring1_idx = (seg + 1) % (isClosed ? numSegments : numSegments + 1);
            
            // For closed revolution, wrap around
            if (isClosed && ring1_idx == 0) {
                ring1_idx = numSegments; // Use the last ring which should match first
            }
            
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

        solid.Stitch();
        return solid;
    }

} // namespace SZM::Geometry
