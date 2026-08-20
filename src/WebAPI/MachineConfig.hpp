#pragma once
#include <string>
#include <array>
#include <nlohmann/json.hpp>

namespace SZM::WebAPI {

struct AxisLimit {
    double travelMm   = 500.0;
    double feedMmMin  = 5000.0;
    double accelMmS2  = 500.0;
};

struct MachineConfig {
    std::string id        = "default";
    std::string name      = "CNC Machine";
    bool        is5Axis   = false;
    std::array<AxisLimit, 3> linear;   // X, Y, Z
    std::array<AxisLimit, 2> rotary;   // A, B (5-axis only)

    nlohmann::json ToJSON() const;
    static MachineConfig FromJSON(const nlohmann::json& j);
};

class MachineConfigRegistry {
public:
    static MachineConfigRegistry& Get();
    MachineConfig       Load(const std::string& id = "default");
    void                Save(const MachineConfig& cfg);
private:
    MachineConfigRegistry() = default;
    std::string FilePath(const std::string& id) const;
};

} // namespace SZM::WebAPI
