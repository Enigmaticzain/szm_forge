#include "ExportToThreeJS.hpp"
#include "../WebAPI/UuidUtils.hpp"
#include <algorithm>
#include <iostream>

namespace SZM::WebViewer {

nlohmann::json ThreeJSExporter::CreateBoxGeometry(float width, float height, float depth) {
    nlohmann::json geom;
    geom["uuid"] = SZM::WebAPI::GenerateUuidV4();
    geom["type"] = "BoxGeometry";
    geom["width"] = width;
    geom["height"] = height;
    geom["depth"] = depth;
    return geom;
}

nlohmann::json ThreeJSExporter::CreateMaterial(const std::string& name, int colorHex, float opacity) {
    nlohmann::json mat;
    mat["uuid"] = SZM::WebAPI::GenerateUuidV4();
    mat["type"] = "MeshStandardMaterial";
    mat["name"] = name;
    mat["color"] = colorHex;
    mat["roughness"] = 0.5f;
    mat["metalness"] = 0.5f;
    mat["opacity"] = opacity;
    mat["transparent"] = opacity < 1.0f;
    return mat;
}

nlohmann::json ThreeJSExporter::ExportScene(const std::vector<SZM::WebAPI::SimulationSnapshot::ComponentSnap>& components) {
    nlohmann::json scene;
    
    // Metadata
    scene["metadata"] = {
        {"version", 4.5},
        {"type", "Object"},
        {"generator", "SZM Forge Three.js Exporter"}
    };

    nlohmann::json geometries = nlohmann::json::array();
    nlohmann::json materials = nlohmann::json::array();
    
    // Create base materials
    nlohmann::json defaultMat = CreateMaterial("DefaultMaterial", 0x888888, 1.0f);
    nlohmann::json dangerMat = CreateMaterial("DangerMaterial", 0xff0000, 1.0f);
    
    materials.push_back(defaultMat);
    materials.push_back(dangerMat);

    // Root object
    nlohmann::json rootObject = {
        {"uuid", SZM::WebAPI::GenerateUuidV4()},
        {"type", "Scene"},
        {"name", "SZM Simulation Scene"},
        {"matrix", {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}},
        {"children", nlohmann::json::array()}
    };

    for (const auto& comp : components) {
        // Derive some simple box bounds from area/thickness (mocking actual boundary box)
        float size = 0.5f; // Hardcoded default for mockup since we don't have area in ComponentSnap
        float depth = 0.5f;
        
        nlohmann::json geom = CreateBoxGeometry(size, size, depth);
        geometries.push_back(geom);

        // Stress colormap (codes/ FEA style): blue → cyan → yellow → red
        const float t = std::min(std::max(comp.stressRatio, 0.0f), 1.2f);
        int colorHex = 0x3366ff;
        if (t < 0.25f) {
            colorHex = 0x1e50c8;
        } else if (t < 0.5f) {
            colorHex = 0x00c8ff;
        } else if (t < 0.8f) {
            colorHex = 0xffaa33;
        } else {
            colorHex = 0xff2200;
        }
        nlohmann::json stressMat = CreateMaterial(comp.name + "_stress", colorHex, 1.0f);
        materials.push_back(stressMat);
        std::string activeMatUuid = stressMat["uuid"].get<std::string>();

        // Create Mesh node
        nlohmann::json mesh;
        mesh["uuid"] = SZM::WebAPI::GenerateUuidV4();
        mesh["type"] = "Mesh";
        mesh["name"] = comp.name;
        mesh["geometry"] = geom["uuid"];
        mesh["material"] = activeMatUuid;
        
        // Use component position (mocking since we only have telemetry)
        mesh["matrix"] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        
        // Add custom user data (telemetry)
        mesh["userData"] = {
            {"stress", comp.stress},
            {"temperature", comp.temperature},
            {"stressRatio", comp.stressRatio},
            {"tempRatio", comp.tempRatio}
        };

        rootObject["children"].push_back(mesh);
    }

    scene["geometries"] = geometries;
    scene["materials"] = materials;
    scene["object"] = rootObject;

    return scene;
}

} // namespace SZM::WebViewer
