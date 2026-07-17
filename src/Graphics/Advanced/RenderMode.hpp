#pragma once

namespace SZM::Graphics {

/**
 * @enum RenderMode
 * @brief Different rendering visualization modes
 */
enum class RenderMode {
    Solid,           ///< Solid shaded rendering
    Wireframe,       ///< Wireframe only
    WireframeShaded, ///< Wireframe overlay on shaded
    FlatShaded,      ///< Flat shading (no interpolation)
    NormalMap,       ///< Normal visualization
    DepthMap         ///< Depth visualization
};

/**\n * @enum LightingMode
n * @brief Different lighting models
 */
enum class LightingMode {
    PBR,             ///< Physically-based rendering
    Phong,           ///< Phong shading
    Flat             ///< Flat/unlit
};

} // namespace SZM::Graphics
