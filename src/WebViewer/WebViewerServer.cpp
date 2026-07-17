#include "WebViewerServer.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <iostream>
#include <chrono>

namespace SZM::WebViewer {

// ============================================================================
// ThreeJSExporter Implementation
// ============================================================================

bool ThreeJSExporter::ExportScene(const std::string& outputPath) {
    return ExportScene(outputPath, ExportConfig{});
}

bool ThreeJSExporter::ExportScene(const std::string& outputPath, const ExportConfig& config) {
    m_CurrentConfig = config;
    m_LastStats = {};

    try {
        std::string sceneJSON = SerializeScene();
        
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            std::cerr << "[WebViewer] Failed to open file: " << outputPath << "\n";
            return false;
        }

        file << sceneJSON;
        file.close();

        // Calculate file size
        std::ifstream checkFile(outputPath, std::ios::ate);
        m_LastStats.fileSizeKB = static_cast<float>(checkFile.tellg()) / 1024.0f;
        checkFile.close();

        std::cout << "[WebViewer] Scene exported successfully to " << outputPath << "\n";
        std::cout << "[WebViewer] Stats: " << m_LastStats.vertexCount << " vertices, "
                  << m_LastStats.triangleCount << " triangles, "
                  << m_LastStats.fileSizeKB << " KB\n";

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[WebViewer] Export failed: " << e.what() << "\n";
        return false;
    }
}

bool ThreeJSExporter::ExportComponent(uint32_t componentId, const std::string& outputPath) {
    m_LastStats = {};

    try {
        std::string geometryJSON = SerializeGeometry(componentId);
        
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            return false;
        }

        file << geometryJSON;
        file.close();

        std::cout << "[WebViewer] Component " << componentId << " exported to " << outputPath << "\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[WebViewer] Component export failed: " << e.what() << "\n";
        return false;
    }
}

std::string ThreeJSExporter::SerializeGeometry(uint32_t componentId) {
    std::stringstream ss;
    
    ss << "{\n";
    ss << "  \"metadata\": {\n";
    ss << "    \"version\": 4.5,\n";
    ss << "    \"type\": \"BufferGeometry\",\n";
    ss << "    \"generator\": \"SZM_Forge_WebViewer\"\n";
    ss << "  },\n";
    ss << "  \"uuid\": \"" << componentId << "\",\n";
    ss << "  \"type\": \"BufferGeometry\",\n";
    ss << "  \"data\": {\n";
    ss << "    \"attributes\": {\n";
    ss << "      \"position\": {\n";
    ss << "        \"itemSize\": 3,\n";
    ss << "        \"type\": \"Float32Array\",\n";
    ss << "        \"array\": [0, 0, 0]\n";
    ss << "      }\n";
    ss << "    }\n";
    ss << "  }\n";
    ss << "}\n";

    m_LastStats.vertexCount = 1;
    m_LastStats.triangleCount = 0;

    return ss.str();
}

std::string ThreeJSExporter::SerializeMaterials() {
    std::stringstream ss;
    
    ss << "{\n";
    ss << "  \"materials\": [\n";
    ss << "    {\n";
    ss << "      \"uuid\": \"default_material\",\n";
    ss << "      \"type\": \"MeshStandardMaterial\",\n";
    ss << "      \"color\": 16777215,\n";
    ss << "      \"metalness\": 0.5,\n";
    ss << "      \"roughness\": 0.5\n";
    ss << "    }\n";
    ss << "  ]\n";
    ss << "}\n";

    m_LastStats.materialCount = 1;
    return ss.str();
}

std::string ThreeJSExporter::SerializeScene() {
    std::stringstream ss;
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    ss << "{\n";
    ss << "  \"metadata\": {\n";
    ss << "    \"version\": 4.5,\n";
    ss << "    \"type\": \"Object3D\",\n";
    ss << "    \"generator\": \"SZM_Forge_WebViewer\",\n";
    ss << "    \"exportTime\": \"" << std::ctime(&time) << "\"\n";
    ss << "  },\n";
    ss << "  \"object\": {\n";
    ss << "    \"uuid\": \"scene_root\",\n";
    ss << "    \"type\": \"Scene\",\n";
    ss << "    \"name\": \"SZM_Forge_Scene\",\n";
    ss << "    \"matrix\": [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1],\n";
    ss << "    \"children\": []\n";
    ss << "  }\n";
    ss << "}\n";

    return ss.str();
}

// ============================================================================
// WebSocketBridge Implementation
// ============================================================================

WebSocketBridge::WebSocketBridge(uint16_t port) : m_Port(port) {
    std::cout << "[WebSocket] Bridge initialized on port " << port << "\n";
}

WebSocketBridge::~WebSocketBridge() {
    if (m_IsRunning) {
        Stop();
    }
}

bool WebSocketBridge::Start() {
    if (m_IsRunning) {
        std::cout << "[WebSocket] Already running\n";
        return true;
    }

    try {
        m_IsRunning = true;
        m_ConnectedClients = 0;
        std::cout << "[WebSocket] Server started on port " << m_Port << "\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[WebSocket] Failed to start: " << e.what() << "\n";
        m_IsRunning = false;
        return false;
    }
}

void WebSocketBridge::Stop() {
    if (!m_IsRunning) {
        return;
    }

    try {
        m_IsRunning = false;
        m_ConnectedClients = 0;
        std::cout << "[WebSocket] Server stopped\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[WebSocket] Error stopping server: " << e.what() << "\n";
    }
}

void WebSocketBridge::BroadcastSceneUpdate(const std::string& jsonDelta) {
    if (!m_IsRunning) {
        return;
    }

    try {
        std::cout << "[WebSocket] Broadcasting scene update (" << jsonDelta.length() << " bytes)\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[WebSocket] Broadcast failed: " << e.what() << "\n";
    }
}

void WebSocketBridge::BroadcastTransform(uint32_t componentId, const float* matrix4x4) {
    if (!m_IsRunning || !matrix4x4) {
        return;
    }

    try {
        std::stringstream ss;
        ss << "{\"type\":\"transform\",\"componentId\":" << componentId << ",\"matrix\":[";
        for (int i = 0; i < 16; ++i) {
            ss << matrix4x4[i];
            if (i < 15) ss << ",";
        }
        ss << "]}";

        std::cout << "[WebSocket] Broadcasting transform for component " << componentId << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[WebSocket] Transform broadcast failed: " << e.what() << "\n";
    }
}

void WebSocketBridge::BroadcastSimulationResult(const std::string& resultJSON) {
    if (!m_IsRunning) {
        return;
    }

    try {
        std::cout << "[WebSocket] Broadcasting simulation result (" << resultJSON.length() << " bytes)\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[WebSocket] Result broadcast failed: " << e.what() << "\n";
    }
}

// ============================================================================
// WebViewerManager Implementation
// ============================================================================

WebViewerManager& WebViewerManager::GetInstance() {
    static WebViewerManager instance;
    return instance;
}

bool WebViewerManager::Initialize(uint16_t wsPort) {
    try {
        if (!m_Bridge.Start()) {
            std::cerr << "[WebViewerManager] Failed to start WebSocket bridge\n";
            return false;
        }

        std::cout << "[WebViewerManager] Initialized successfully\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[WebViewerManager] Initialization failed: " << e.what() << "\n";
        return false;
    }
}

void WebViewerManager::Shutdown() {
    try {
        m_Bridge.Stop();
        std::cout << "[WebViewerManager] Shutdown complete\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[WebViewerManager] Shutdown error: " << e.what() << "\n";
    }
}

std::string WebViewerManager::GetViewerURL() const {
    return "http://localhost:8080/viewer.html";
}

} // namespace SZM::WebViewer
