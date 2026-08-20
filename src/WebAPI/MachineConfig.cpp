#include "MachineConfig.hpp"
#include <fstream>
#include <filesystem>

namespace SZM::WebAPI {

static nlohmann::json axisToJson(const AxisLimit& a) {
    return { {"travel_mm", a.travelMm}, {"feed_mm_min", a.feedMmMin}, {"accel_mm_s2", a.accelMmS2} };
}

static AxisLimit axisFromJson(const nlohmann::json& j) {
    AxisLimit a;
    a.travelMm  = j.value("travel_mm",   a.travelMm);
    a.feedMmMin = j.value("feed_mm_min", a.feedMmMin);
    a.accelMmS2 = j.value("accel_mm_s2", a.accelMmS2);
    return a;
}

nlohmann::json MachineConfig::ToJSON() const {
    nlohmann::json j;
    j["id"]       = id;
    j["name"]     = name;
    j["is_5axis"] = is5Axis;
    j["linear"]   = { axisToJson(linear[0]), axisToJson(linear[1]), axisToJson(linear[2]) };
    j["rotary"]   = { axisToJson(rotary[0]), axisToJson(rotary[1]) };
    return j;
}

MachineConfig MachineConfig::FromJSON(const nlohmann::json& j) {
    MachineConfig cfg;
    cfg.id      = j.value("id",       cfg.id);
    cfg.name    = j.value("name",     cfg.name);
    cfg.is5Axis = j.value("is_5axis", cfg.is5Axis);
    if (j.contains("linear") && j["linear"].is_array()) {
        for (size_t i = 0; i < 3 && i < j["linear"].size(); ++i)
            cfg.linear[i] = axisFromJson(j["linear"][i]);
    }
    if (j.contains("rotary") && j["rotary"].is_array()) {
        for (size_t i = 0; i < 2 && i < j["rotary"].size(); ++i)
            cfg.rotary[i] = axisFromJson(j["rotary"][i]);
    }
    return cfg;
}

MachineConfigRegistry& MachineConfigRegistry::Get() {
    static MachineConfigRegistry inst;
    return inst;
}

std::string MachineConfigRegistry::FilePath(const std::string& id) const {
    std::filesystem::create_directories("machine_data");
    return "machine_data/machine_" + id + ".json";
}

MachineConfig MachineConfigRegistry::Load(const std::string& id) {
    std::ifstream f(FilePath(id));
    if (!f.is_open()) return MachineConfig{};
    try {
        return MachineConfig::FromJSON(nlohmann::json::parse(f));
    } catch (...) {
        return MachineConfig{};
    }
}

void MachineConfigRegistry::Save(const MachineConfig& cfg) {
    std::ofstream f(FilePath(cfg.id));
    f << cfg.ToJSON().dump(2);
}

} // namespace SZM::WebAPI
