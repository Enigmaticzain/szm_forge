#include "ThreeJSRenderer.hpp"
#include "../Materials/MaterialDatabase.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include <fstream>
#include <cmath>
#include <chrono>
#include <nlohmann/json.hpp>

namespace SZM::WebViewer {

ThreeJSRenderer::ThreeJSRenderer()
    : ThreeJSRenderer(RenderConfig{}) {}

ThreeJSRenderer::ThreeJSRenderer(const RenderConfig& config)
    : m_RenderConfig(config) {}

bool ThreeJSRenderer::ExportScene(const std::string& outputPath, 
                                  const RenderConfig* config) {
    auto start = std::chrono::high_resolution_clock::now();
    if (config) {
        m_RenderConfig = *config;
    }

    json geometries = json::array();
    json materials = json::array();
    json children = json::array();

    // Default ambient & directional light
    children.push_back({
        {"uuid", "light-ambient-01"},
        {"type", "AmbientLight"},
        {"name", "AmbientLight"},
        {"color", 0xffffff},
        {"intensity", 0.6}
    });

    children.push_back({
        {"uuid", "light-dir-01"},
        {"type", "DirectionalLight"},
        {"name", "KeyLight"},
        {"color", 0xffffff},
        {"intensity", 0.8},
        {"matrix", {1,0,0,0, 0,1,0,0, 0,0,1,0, 5,10,7,1}}
    });

    auto& engine = SZM::SimulationEngine::GetInstance();
    const auto& components = engine.GetComponents();

    for (const auto& comp : components) {
        if (!comp) continue;
        json geom = ConvertMeshToThreeJS(comp->id);
        json mat = ConvertMaterialToThreeJS(comp->materialName.empty() ? comp->materialId : comp->materialName);

        geometries.push_back(geom);
        materials.push_back(mat);

        json meshNode = {
            {"uuid", "mesh-" + std::to_string(comp->id)},
            {"type", "Mesh"},
            {"name", comp->name},
            {"geometry", geom["uuid"]},
            {"material", mat["uuid"]},
            {"matrix", {1,0,0,0, 0,1,0,0, 0,0,1,0,
                        comp->position.x, comp->position.y, comp->position.z, 1}},
            {"userData", {
                {"componentId", comp->id},
                {"stress_MPa", comp->stress / 1e6f},
                {"temperature_K", comp->temperature},
                {"isDangerous", comp->isDangerous}
            }}
        };
        children.push_back(meshNode);
    }

    json sceneData = {
        {"metadata", {
            {"version", 4.5},
            {"type", "Object"},
            {"generator", "SZM Forge ThreeJSRenderer"}
        }},
        {"geometries", geometries},
        {"materials", materials},
        {"object", {
            {"uuid", "scene-root-01"},
            {"type", "Scene"},
            {"name", "SZM Forge Assembly"},
            {"matrix", {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}},
            {"children", children}
        }}
    };

    bool success = WriteJSON(outputPath, sceneData);

    auto end = std::chrono::high_resolution_clock::now();
    m_LastStats.exportTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    m_LastStats.materialCount = static_cast<uint32_t>(materials.size());
    m_LastStats.triangleCount = static_cast<uint32_t>(geometries.size() * 12);
    m_LastStats.vertexCount = static_cast<uint32_t>(geometries.size() * 8);

    return success;
}

bool ThreeJSRenderer::ExportComponent(uint32_t componentId, 
                                      const std::string& outputPath,
                                      bool includePhysics) {
    auto* comp = SZM::SimulationEngine::GetInstance().GetComponent(componentId);
    if (!comp) return false;

    json geom = ConvertMeshToThreeJS(componentId);
    json mat = ConvertMaterialToThreeJS(comp->materialName.empty() ? comp->materialId : comp->materialName);

    json compData = {
        {"metadata", {
            {"version", 4.5},
            {"type", "Component"},
            {"generator", "SZM Forge"}
        }},
        {"geometry", geom},
        {"material", mat},
        {"component", {
            {"id", comp->id},
            {"name", comp->name},
            {"area", comp->area},
            {"thickness", comp->thickness}
        }}
    };

    if (includePhysics) {
        compData["physics"] = ConvertPhysicsToThreeJS(componentId);
    }

    return WriteJSON(outputPath, compData);
}

bool ThreeJSRenderer::ExportFEAResults(const json& fea_data,
                                       uint32_t componentId,
                                       const std::string& outputPath,
                                       const std::string& resultType) {
    json feaExport = {
        {"componentId", componentId},
        {"resultType", resultType},
        {"data", fea_data},
        {"colormap", {
            {"0.00", "#1e50c8"},
            {"0.25", "#00c8ff"},
            {"0.50", "#22ee44"},
            {"0.75", "#ffaa33"},
            {"1.00", "#ff2200"}
        }}
    };
    return WriteJSON(outputPath, feaExport);
}

bool ThreeJSRenderer::GenerateHTMLViewer(const std::string& sceneJsonPath,
                                         const std::string& outputHtmlPath,
                                         const json& options) {
    std::ofstream html(outputHtmlPath);
    if (!html) return false;

    html << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n"
         << "<head>\n"
         << "  <meta charset=\"UTF-8\">\n"
         << "  <title>SZM Forge 3D Viewer</title>\n"
         << "  <style>\n"
         << "    body { margin: 0; overflow: hidden; background: #111; font-family: sans-serif; }\n"
         << "    #info { position: absolute; top: 10px; left: 10px; color: #fff; background: rgba(0,0,0,0.6); padding: 8px 14px; border-radius: 4px; }\n"
         << "  </style>\n"
         << "  <script src=\"https://cdn.jsdelivr.net/npm/three@0.128.0/build/three.min.js\"></script>\n"
         << "  <script src=\"https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/controls/OrbitControls.js\"></script>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <div id=\"info\">SZM Forge 3D Web Viewer &bull; Orbit: Left Click &bull; Pan: Right Click</div>\n"
         << "  <script>\n"
         << "    const scene = new THREE.Scene();\n"
         << "    scene.background = new THREE.Color(0x1a1a24);\n"
         << "    const camera = new THREE.PerspectiveCamera(60, window.innerWidth / window.innerHeight, 0.1, 1000);\n"
         << "    camera.position.set(3, 3, 5);\n"
         << "    const renderer = new THREE.WebGLRenderer({ antialias: true });\n"
         << "    renderer.setSize(window.innerWidth, window.innerHeight);\n"
         << "    document.body.appendChild(renderer.domElement);\n"
         << "    const controls = new THREE.OrbitControls(camera, renderer.domElement);\n"
         << "    const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);\n"
         << "    scene.add(ambientLight);\n"
         << "    const dirLight = new THREE.DirectionalLight(0xffffff, 0.8);\n"
         << "    dirLight.position.set(10, 20, 15);\n"
         << "    scene.add(dirLight);\n"
         << "    const grid = new THREE.GridHelper(10, 20, 0x00ffcc, 0x444455);\n"
         << "    scene.add(grid);\n"
         << "    fetch('" << sceneJsonPath << "')\n"
         << "      .then(r => r.json())\n"
         << "      .then(obj => {\n"
         << "        const loader = new THREE.ObjectLoader();\n"
         << "        const loaded = loader.parse(obj);\n"
         << "        scene.add(loaded);\n"
         << "      })\n"
         << "      .catch(e => console.warn('Loading fallback scene geometry'));\n"
         << "    function animate() {\n"
         << "      requestAnimationFrame(animate);\n"
         << "      controls.update();\n"
         << "      renderer.render(scene, camera);\n"
         << "    }\n"
         << "    animate();\n"
         << "  </script>\n"
         << "</body>\n"
         << "</html>\n";
    return true;
}

bool ThreeJSRenderer::ExportMaterialLibrary(const std::string& outputPath) {
    auto& db = Materials::MaterialDatabase::GetInstance();
    auto materials = db.GetAllMaterials();
    json matList = json::array();
    for (const auto& m : materials) {
        matList.push_back(ConvertMaterialToThreeJS(m.name));
    }
    return WriteJSON(outputPath, matList);
}

json ThreeJSRenderer::ConvertMeshToThreeJS(uint32_t componentId) {
    auto* comp = SZM::SimulationEngine::GetInstance().GetComponent(componentId);
    float width = 0.5f;
    float height = 0.2f;
    float depth = 0.5f;
    if (comp) {
        width = std::max(0.05f, std::sqrt(comp->area));
        height = std::max(0.01f, comp->thickness);
        depth = width * 0.8f;
    }

    return {
        {"uuid", "geom-" + std::to_string(componentId)},
        {"type", "BoxGeometry"},
        {"width", width},
        {"height", height},
        {"depth", depth}
    };
}

json ThreeJSRenderer::ConvertMaterialToThreeJS(const std::string& materialName) {
    auto& db = Materials::MaterialDatabase::GetInstance();
    auto mat = db.GetMaterial(materialName);
    int color = 0x8899aa;
    float roughness = 0.4f;
    float metalness = 0.6f;

    if (mat) {
        if (mat->id.find("STEEL") != std::string::npos) {
            color = 0x9098a0;
            roughness = 0.35f;
            metalness = 0.85f;
        } else if (mat->id.find("AL") != std::string::npos) {
            color = 0xd0d5dd;
            roughness = 0.3f;
            metalness = 0.9f;
        } else if (mat->id.find("CU") != std::string::npos || mat->id.find("COPPER") != std::string::npos) {
            color = 0xc87d55;
            roughness = 0.25f;
            metalness = 0.95f;
        } else if (mat->id.find("TI") != std::string::npos) {
            color = 0x6e737c;
            roughness = 0.4f;
            metalness = 0.8f;
        }
    }

    return {
        {"uuid", "mat-" + materialName},
        {"type", "MeshStandardMaterial"},
        {"name", materialName},
        {"color", color},
        {"roughness", roughness},
        {"metalness", metalness}
    };
}

json ThreeJSRenderer::ConvertPhysicsToThreeJS(uint32_t componentId) {
    auto* comp = SZM::SimulationEngine::GetInstance().GetComponent(componentId);
    float mass = 1.0f;
    if (comp) {
        mass = std::max(0.01f, comp->area * comp->thickness * (comp->density > 0.0f ? comp->density : 7850.0f));
    }
    return {
        {"type", "box"},
        {"mass", mass},
        {"friction", 0.3f},
        {"restitution", 0.1f}
    };
}

json ThreeJSRenderer::GenerateLODData(const json& highPolyMesh) {
    json lod = json::array();
    lod.push_back({{"distance", 0}, {"geometry", highPolyMesh}});
    return lod;
}

bool ThreeJSRenderer::WriteGLTF(const std::string& outputPath, const json& sceneData) {
    return WriteJSON(outputPath, sceneData);
}

bool ThreeJSRenderer::WriteJSON(const std::string& outputPath, const json& sceneData) {
    std::ofstream file(outputPath);
    if (!file) return false;
    file << sceneData.dump(2);
    return true;
}

} // namespace SZM::WebViewer
