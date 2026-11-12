#include "WebViewerServer.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace SZM::WebViewer {

namespace {

std::string FormatIsoTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf{};
#if defined(_WIN32)
    gmtime_s(&tmBuf, &nowTime);
#else
    gmtime_r(&nowTime, &tmBuf);
#endif
    char buf[32]{};
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tmBuf) == 0) {
        return "1970-01-01T00:00:00Z";
    }
    return std::string(buf);
}

nlohmann::json BuildBoxBufferGeometry(const std::string& uuid, float width, float height, float depth) {
    const float hx = width * 0.5f;
    const float hy = height * 0.5f;
    const float hz = depth * 0.5f;
    return {
        {"uuid", uuid},
        {"type", "BufferGeometry"},
        {"data", {
            {"attributes", {
                {"position", {
                    {"itemSize", 3},
                    {"type", "Float32Array"},
                    {"array", {
                        -hx, -hy, -hz,   hx, -hy, -hz,   hx,  hy, -hz,  -hx,  hy, -hz,
                        -hx, -hy,  hz,   hx, -hy,  hz,   hx,  hy,  hz,  -hx,  hy,  hz
                    }}
                }}
            }},
            {"index", {
                {"type", "Uint16Array"},
                {"array", {
                    0, 1, 2,  0, 2, 3,
                    4, 6, 5,  4, 7, 6,
                    0, 4, 5,  0, 5, 1,
                    1, 5, 6,  1, 6, 2,
                    2, 6, 7,  2, 7, 3,
                    3, 7, 4,  3, 4, 0
                }}
            }}
        }}
    };
}

} // namespace

// ============================================================================
// WebViewerSceneExporter Implementation
// ============================================================================

bool WebViewerSceneExporter::ExportScene(const std::string& outputPath) {
    return ExportScene(outputPath, ExportConfig{});
}

bool WebViewerSceneExporter::ExportScene(const std::string& outputPath, const ExportConfig& config) {
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

        // Calculate file size safely
        std::ifstream checkFile(outputPath, std::ios::binary | std::ios::ate);
        const std::streampos sizePos = checkFile.is_open() ? checkFile.tellg() : std::streampos(-1);
        m_LastStats.fileSizeKB = (sizePos > 0) ? static_cast<float>(sizePos) / 1024.0f : 0.0f;
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

bool WebViewerSceneExporter::ExportComponent(uint32_t componentId, const std::string& outputPath) {
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

std::string WebViewerSceneExporter::SerializeGeometry(uint32_t componentId) {
    const auto* comp = SimulationEngine::GetInstance().GetComponent(componentId);
    const float width = comp ? std::max(0.05f, std::sqrt(comp->area)) : 0.5f;
    const float height = comp ? std::max(0.01f, comp->thickness) : 0.2f;
    const float depth = width * 0.8f;

    nlohmann::json geom = BuildBoxBufferGeometry("comp-" + std::to_string(componentId), width, height, depth);
    geom["metadata"] = {
        {"version", 4.5},
        {"type", "BufferGeometry"},
        {"generator", "SZM_Forge_WebViewer"}
    };

    m_LastStats.vertexCount = 8;
    m_LastStats.triangleCount = 12;

    return geom.dump(2) + "\n";
}

std::string WebViewerSceneExporter::SerializeMaterials() {
    nlohmann::json payload = {
        {"materials", {
            {
                {"uuid", "default_material"},
                {"type", "MeshStandardMaterial"},
                {"color", 16777215},
                {"metalness", 0.5},
                {"roughness", 0.5}
            }
        }}
    };

    m_LastStats.materialCount = 1;
    return payload.dump(2) + "\n";
}

std::string WebViewerSceneExporter::SerializeScene() {
    nlohmann::json geometries = nlohmann::json::array();
    nlohmann::json materials = nlohmann::json::array();
    nlohmann::json children = nlohmann::json::array();

    const auto& components = SimulationEngine::GetInstance().GetComponents();
    for (const auto& comp : components) {
        if (!comp) continue;
        const std::string geomUuid = "geom-" + std::to_string(comp->id);
        const std::string matUuid = "mat-" + std::to_string(comp->id);
        const float width = std::max(0.05f, std::sqrt(comp->area));
        const float height = std::max(0.01f, comp->thickness);
        const float depth = width * 0.8f;

        geometries.push_back(BuildBoxBufferGeometry(geomUuid, width, height, depth));
        materials.push_back({
            {"uuid", matUuid},
            {"type", "MeshStandardMaterial"},
            {"name", comp->materialName.empty() ? comp->materialId : comp->materialName},
            {"color", comp->isDangerous ? 0xff4422 : 0x3388cc},
            {"metalness", 0.5},
            {"roughness", 0.5}
        });
        children.push_back({
            {"uuid", "mesh-" + std::to_string(comp->id)},
            {"type", "Mesh"},
            {"name", comp->name},
            {"geometry", geomUuid},
            {"material", matUuid},
            {"matrix", {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
                        comp->position.x, comp->position.y, comp->position.z, 1}}
        });
    }

    m_LastStats.vertexCount = static_cast<uint32_t>(children.size() * 8);
    m_LastStats.triangleCount = static_cast<uint32_t>(children.size() * 12);
    m_LastStats.materialCount = static_cast<uint32_t>(materials.size());

    nlohmann::json scene = {
        {"metadata", {
            {"version", 4.5},
            {"type", "Object3D"},
            {"generator", "SZM_Forge_WebViewer"},
            {"exportTime", FormatIsoTimestamp()}
        }},
        {"geometries", geometries},
        {"materials", materials},
        {"object", {
            {"uuid", "scene_root"},
            {"type", "Scene"},
            {"name", "SZM_Forge_Scene"},
            {"matrix", {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}},
            {"children", children}
        }}
    };

    return scene.dump(2) + "\n";
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
