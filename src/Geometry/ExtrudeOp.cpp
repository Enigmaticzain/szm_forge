#include "operations/ExtrudeOp.hpp"
#include <numeric>
#include <cmath>

namespace SZM::Geometry::Operations {

    SolidBody LinearExtrude(const SketchProfile& profile, const ExtrudeParams& params) {
        if (!profile.IsValid()) {
            throw std::invalid_argument("SZM_GEOM_ERROR: Profile must have at least 1 valid loop.");
        }

        SolidBody result;
        
        // For Phase 1, we only handle the outer loop (first loop)
        const std::vector<Vector3>& loop = profile.Loops[0];
        const size_t vertexCount = loop.size();
        
        Vector3 extrudeDir = params.Direction.Magnitude() > 0.0 ? params.Direction.Normalize() : profile.PlaneNormal.Normalize();
        Vector3 translation = extrudeDir * params.Distance;

        // Calculate centroid for taper
        Vector3 centroid;
        for (const auto& v : loop) {
            centroid.x += v.x; centroid.y += v.y; centroid.z += v.z;
        }
        centroid = centroid * (1.0 / static_cast<double>(vertexCount));

        bool hasTaper = std::abs(params.DraftAngleRad) > 1e-6;
        double taperScale = 1.0;

        if (hasTaper) {
            double avgRadius = 0.0;
            for (const auto& v : loop) {
                avgRadius += (v - centroid).Magnitude();
            }
            avgRadius /= vertexCount;

            double draftOffset = params.Distance * std::tan(params.DraftAngleRad);
            taperScale = 1.0 - (draftOffset / avgRadius); 
        }

        // Collect all vertices (bottom ring + top ring)
        std::vector<Vector3> allVertices;
        allVertices.reserve(vertexCount * 2);
        
        // Bottom ring
        for (const auto& v : loop) {
            allVertices.push_back(v);
        }

        // Top ring
        for (const auto& v : loop) {
            Vector3 vNew = v;
            if (hasTaper) {
                vNew = centroid + (v - centroid) * taperScale;
            }
            vNew = vNew + translation;
            allVertices.push_back(vNew);
        }

        // Bottom cap (reversed winding)
        std::vector<Vector3> bottomCap;
        for (size_t i = vertexCount; i > 0; --i) {
            bottomCap.push_back(allVertices[i - 1]);
        }
        result.AddFace(bottomCap);

        // Top cap
        std::vector<Vector3> topCap;
        for (size_t i = 0; i < vertexCount; ++i) {
            topCap.push_back(allVertices[vertexCount + i]);
        }
        result.AddFace(topCap);

        // Side faces
        for (size_t i = 0; i < vertexCount; ++i) {
            size_t next_i = (i + 1) % vertexCount;
            
            std::vector<Vector3> quad = {
                allVertices[i],
                allVertices[next_i],
                allVertices[next_i + vertexCount],
                allVertices[i + vertexCount]
            };
            
            // Split quad into two triangles
            std::vector<Vector3> tri1 = {quad[0], quad[1], quad[2]};
            std::vector<Vector3> tri2 = {quad[0], quad[2], quad[3]};
            
            result.AddFace(tri1);
            result.AddFace(tri2);
        }

        result.Stitch();
        return result;
    }

} // namespace SZM::Geometry::Operations
