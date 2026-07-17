#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cstdint>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace SZM::WebViewer {

    using json = nlohmann::json;

    /**
     * @brief High-performance Three.js scene renderer
     * Exports SZM scenes to Three.js compatible format with:
     * - Real-time physics visualization
     * - Material & lighting systems (PBR)
     * - Interactive scene graph
     * - Streaming/LOD support
     */
    class ThreeJSRenderer {
    public:
        struct RenderConfig {
            bool usePhysics = true;
            bool usePhysicsWorker = true;  // Use Cannon.js or similar
            bool streamLOD = true;
            float lodThreshold = 0.1f;
            bool compressGeometry = true;
            uint32_t maxLODLevels = 4;
            bool enableShadows = true;
            bool enableSSAO = true;
            std::string outputFormat = "gltf";  // gltf, glb, json
        };

        struct PhysicsConfig {
            bool enabled = true;
            std::string engine = "cannon-es";  // cannon-es or Babylon physics
            float gravity = -9.81f;
            float timeStep = 1.0f / 60.0f;
            uint32_t iterations = 3;
        };

        struct ExportStats {
            uint32_t vertexCount = 0;
            uint32_t triangleCount = 0;
            uint32_t materialCount = 0;
            uint32_t lightCount = 0;
            uint32_t rigidBodyCount = 0;
            float fileSizeKB = 0.0f;
            double exportTimeMs = 0.0;
        };

        ThreeJSRenderer();
        explicit ThreeJSRenderer(const RenderConfig& config);
        ~ThreeJSRenderer() = default;

        /**
         * @brief Export complete scene to Three.js format
         * @param outputPath Output file path (.gltf, .glb, or .json)
         * @param config Optional render configuration override
         * @return true if successful
         */
        bool ExportScene(const std::string& outputPath, 
                        const RenderConfig* config = nullptr);

        /**
         * @brief Export component with physics
         * @param componentId Component UUID
         * @param outputPath Output file path
         * @param includePhysics Include rigid body data
         * @return true if successful
         */
        bool ExportComponent(uint32_t componentId, 
                            const std::string& outputPath,
                            bool includePhysics = true);

        /**
         * @brief Export FEA results as Three.js visualization
         * @param fea_data FEA solution data (stress, displacement, etc)
         * @param componentId Target component
         * @param outputPath Output file
         * @param resultType "stress", "displacement", "thermal"
         * @return true if successful
         */
        bool ExportFEAResults(const json& fea_data,
                             uint32_t componentId,
                             const std::string& outputPath,
                             const std::string& resultType);

        /**
         * @brief Create HTML viewer wrapper
         * @param sceneJsonPath Path to exported scene
         * @param outputHtmlPath HTML output path
         * @param options View options (lights, camera, controls, etc)
         * @return true if successful
         */
        bool GenerateHTMLViewer(const std::string& sceneJsonPath,
                               const std::string& outputHtmlPath,
                               const json& options = json::object());

        /**
         * @brief Get export statistics from last operation
         */
        const ExportStats& GetStats() const { return m_LastStats; }

        /**
         * @brief Set physics configuration
         */
        void SetPhysicsConfig(const PhysicsConfig& config) { m_PhysicsConfig = config; }

        /**
         * @brief Generate material library (PBR + custom)
         * @param outputPath Path to materials JSON
         * @return true if successful
         */
        bool ExportMaterialLibrary(const std::string& outputPath);

        /**
         * @brief Enable/disable LOD generation
         */
        void SetLODEnabled(bool enabled) { m_RenderConfig.streamLOD = enabled; }

        /**
         * @brief Set compression level (0-9)
         */
        void SetCompressionLevel(uint32_t level) { 
            m_CompressionLevel = std::min(level, 9u); 
        }

    private:
        RenderConfig m_RenderConfig;
        PhysicsConfig m_PhysicsConfig;
        ExportStats m_LastStats;
        uint32_t m_CompressionLevel = 6;

        // Helper methods
        json ConvertMeshToThreeJS(uint32_t componentId);
        json ConvertMaterialToThreeJS(const std::string& materialName);
        json ConvertPhysicsToThreeJS(uint32_t componentId);
        json GenerateLODData(const json& highPolyMesh);
        bool WriteGLTF(const std::string& outputPath, const json& sceneData);
        bool WriteJSON(const std::string& outputPath, const json& sceneData);
    };

}
