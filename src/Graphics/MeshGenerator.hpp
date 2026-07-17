#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
#include <numbers>
#include <algorithm>

// Forward declaration — avoids pulling the full B-Rep header into GPU code
namespace SZM::Geometry { class SolidBody; }

namespace SZM::Graphics {

    // ----------------------------------------------------------------
    //  GPU vertex layout — 32-byte aligned for optimal streaming
    // ----------------------------------------------------------------
    struct RenderVertex {
        float Position[3];  ///< World-space XYZ
        float Normal[3];    ///< Smooth per-vertex normal (not face normal)
        float TexCoords[2]; ///< UV [0..1]
    };

    // ----------------------------------------------------------------
    //  GPU mesh — VRAM handles populated by GraphicsContext after upload
    // ----------------------------------------------------------------
    struct RenderMesh {
        std::vector<RenderVertex> Vertices;
        std::vector<uint32_t>     Indices;

        uint32_t VAO = 0; ///< Vertex Array Object  (OpenGL)
        uint32_t VBO = 0; ///< Vertex Buffer Object (OpenGL)
        uint32_t IBO = 0; ///< Index  Buffer Object (OpenGL)
    };

    // ----------------------------------------------------------------
    //  Tessellation configuration
    // ----------------------------------------------------------------
    struct TessellationConfig {
        double MaxChordalError = 0.01;  ///< ε max deviation from true curve [m]
        bool   EnableDynamicLOD = true; ///< Scale ε by camera distance
        double CameraDistance   = 1.0;  ///< Distance from camera [m]
    };

    // ----------------------------------------------------------------
    //  MeshGenerator — converts mathematical B-Rep → GPU triangle mesh
    // ----------------------------------------------------------------
    class MeshGenerator {
    public:
        /**
         * @brief Tessellate a SolidBody into a GPU-ready RenderMesh.
         *
         * Pipeline:
         *   1. Planar faces  → Ear-clipping / Delaunay triangulation
         *   2. Curved faces  → Parametric UV grid via CalculateSegmentCount
         *   3. Vertex normals → Averaged over connected faces (smooth shading)
         *
         * @note VRAM upload (VAO/VBO/IBO) is handled separately by GraphicsContext.
         */
        [[nodiscard]] static RenderMesh TessellateSolid(
            const SZM::Geometry::SolidBody& body,
            const TessellationConfig&       config
        );

        /**
         * @brief Calculate the segment count needed to approximate a circular arc
         *        within a given chordal error tolerance.
         *
         * Formula: N ≈ π / arccos(1 − ε/R)
         *
         * @param radius       True radius of the arc/circle [m]
         * @param chordalError Maximum allowable deviation ε [m]
         * @return Segment count, clamped to [3, 1024]
         */
        [[nodiscard]] static uint32_t CalculateSegmentCount(
            double radius,
            double chordalError
        );
    };

} // namespace SZM::Graphics
