#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace SZM::WebViewer {

    using json = nlohmann::json;

    /**
     * @brief Babylon.js 3D renderer (alternative to Three.js)
     * Features:
     * - Advanced post-processing
     * - Built-in physics (Cannon.js)
     * - Better particle systems
     * - Real-time ray tracing
     * - Native glTF 2.0 support
     */
    class BabylonJSRenderer {
    public:
        struct RenderConfig {
            bool enablePhysics = true;
            bool enablePostProcessing = true;
            bool enableRayTracing = false;
            bool enableShadowMap = true;
            float shadowMapResolution = 2048.0f;
            bool enableSSAO = true;
            bool enableMotionBlur = false;
            std::string physicsEngine = "cannon";  // "cannon" or "oimo"
        };

        struct PhysicsConfig {
            float gravity = -9.81f;
            std::string solver = "cannon";
        };

        BabylonJSRenderer(const RenderConfig& config = {});
        ~BabylonJSRenderer() = default;

        /**
         * @brief Export scene to Babylon.js format
         */
        bool ExportScene(const std::string& outputPath);

        /**
         * @brief Generate HTML viewer with Babylon.js
         */
        bool GenerateHTMLViewer(const std::string& sceneJsonPath,
                               const std::string& outputHtmlPath);

        /**
         * @brief Export with advanced post-processing
         */
        bool ExportWithPostProcessing(const std::string& outputPath,
                                     const std::string& effectName);

        /**
         * @brief Enable ray tracing export (Babylon.js ray tracing)
         */
        bool ExportWithRayTracing(const std::string& outputPath);

    private:
        RenderConfig m_RenderConfig;
        PhysicsConfig m_PhysicsConfig;
    };

    /**
     * @brief Renderer factory - switch between Three.js and Babylon.js
     */
    class RendererFactory {
    public:
        enum class RendererType { ThreeJS, BabylonJS };

        static std::unique_ptr<ThreeJSRenderer> CreateThreeJSRenderer();
        static std::unique_ptr<BabylonJSRenderer> CreateBabylonJSRenderer();

        /**
         * @brief Export to multiple formats at once
         */
        static bool ExportMultiple(const std::vector<RendererType>& renderers,
                                  const std::string& outputDir);
    };

}
