#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "../Simulation/SimulationEngine.hpp"

#include "../WebAPI/FastAPIBridge.hpp"

namespace SZM::WebViewer {

/**
 * @class ThreeJSExporter
 * @brief Exports SZM Forge internal geometry and simulation states to Three.js Object3D JSON format
 */
class ThreeJSExporter {
public:
    /**
     * @brief Serialize a list of simulation components into a full Three.js Scene JSON
     */
    static nlohmann::json ExportScene(const std::vector<SZM::WebAPI::SimulationSnapshot::ComponentSnap>& components);

private:
    static nlohmann::json CreateBoxGeometry(float width, float height, float depth);
    static nlohmann::json CreateMaterial(const std::string& name, int colorHex, float opacity);
};

} // namespace SZM::WebViewer
