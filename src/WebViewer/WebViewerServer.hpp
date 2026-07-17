#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace SZM::WebViewer {

    /**
     * @brief Exports SZM scene to Three.js JSON format
     * Enables real-time 3D visualization in web browsers
     */
    class ThreeJSExporter {
    public:
        struct ExportConfig {
            bool includePhysics = true;
            bool includeMaterials = true;
            bool includeAnimations = false;
            bool compressGeometry = true;
            float lodThreshold = 0.1f;
        };

        ThreeJSExporter() = default;
        ~ThreeJSExporter() = default;

        /**
         * @brief Export current scene to Three.js format
         * @param outputPath Path to write JSON file
         * @param config Export configuration
         * @return true if successful
         */
        bool ExportScene(const std::string& outputPath, const ExportConfig& config);
        bool ExportScene(const std::string& outputPath);

        /**
         * @brief Export single component to Three.js mesh
         * @param componentId Component UUID
         * @param outputPath Path to write JSON
         * @return true if successful
         */
        bool ExportComponent(uint32_t componentId, const std::string& outputPath);

        /**
         * @brief Get export statistics
         */
        struct Stats {
            uint32_t vertexCount = 0;
            uint32_t triangleCount = 0;
            uint32_t materialCount = 0;
            float fileSizeKB = 0.0f;
        };
        Stats GetLastExportStats() const { return m_LastStats; }

    private:
        Stats m_LastStats;
        ExportConfig m_CurrentConfig;

        std::string SerializeGeometry(uint32_t componentId);
        std::string SerializeMaterials();
        std::string SerializeScene();
    };

    /**
     * @brief WebSocket server for real-time scene synchronization
     * Streams updates to connected web clients
     */
    class WebSocketBridge {
    public:
        WebSocketBridge(uint16_t port = 8080);
        ~WebSocketBridge();

        bool Start();
        void Stop();
        bool IsRunning() const { return m_IsRunning; }

        /**
         * @brief Broadcast scene update to all connected clients
         */
        void BroadcastSceneUpdate(const std::string& jsonDelta);

        /**
         * @brief Broadcast component transformation
         */
        void BroadcastTransform(uint32_t componentId, const float* matrix4x4);

        /**
         * @brief Broadcast simulation result
         */
        void BroadcastSimulationResult(const std::string& resultJSON);

        uint32_t GetConnectedClients() const { return m_ConnectedClients; }

    private:
        uint16_t m_Port;
        bool m_IsRunning = false;
        uint32_t m_ConnectedClients = 0;
        void* m_WebSocketContext = nullptr;
    };

    /**
     * @brief Manages web viewer lifecycle
     */
    class WebViewerManager {
    public:
        static WebViewerManager& GetInstance();

        bool Initialize(uint16_t wsPort = 8080);
        void Shutdown();

        ThreeJSExporter& GetExporter() { return m_Exporter; }
        WebSocketBridge& GetBridge() { return m_Bridge; }

        /**
         * @brief Enable real-time sync of scene changes
         */
        void EnableRealtimeSync(bool enable) { m_RealtimeSyncEnabled = enable; }

        /**
         * @brief Get web viewer URL
         */
        std::string GetViewerURL() const;

    private:
        WebViewerManager() : m_Bridge(8080) {}
        ~WebViewerManager() = default;

        ThreeJSExporter m_Exporter;
        WebSocketBridge m_Bridge;
        bool m_RealtimeSyncEnabled = false;
    };

} // namespace SZM::WebViewer
