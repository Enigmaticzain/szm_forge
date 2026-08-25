#include "BridgeClient.hpp"

#include <httplib.h>
#include <map>
#include <set>

namespace SZM {

namespace {

httplib::Client MakeClient(uint16_t port) {
    httplib::Client cli(SZM::BridgeClient::DefaultHost, port);
    cli.set_connection_timeout(2, 0);
    cli.set_read_timeout(30, 0);
    return cli;
}

} // namespace

bool BridgeClient::IsReachable(uint16_t port) {
    return GetJson("/stack/status", port).has_value() || GetJson("/stack/health", port).has_value();
}

std::optional<nlohmann::json> BridgeClient::GetJson(const std::string& path, uint16_t port) {
    auto cli = MakeClient(port);
    auto res = cli.Get(path.c_str());
    if (!res || res->status != 200) {
        return std::nullopt;
    }
    try {
        return nlohmann::json::parse(res->body);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<BridgeClient::StackStatus> BridgeClient::FetchStackStatus(uint16_t port) {
    auto j = GetJson("/stack/status", port);
    if (!j) {
        j = GetJson("/stack/health", port);
    }
    if (!j) {
        StackStatus s;
        s.connected = false;
        s.lastError = "Bridge unreachable on port " + std::to_string(port);
        return s;
    }
    StackStatus s;
    s.raw = *j;
    s.connected = true;
    s.available = j->value("available", 0);
    s.totalTools = j->value("total_tools", 0);
    s.status = j->value("status", "unknown");
    ApplyReadyFlags(s, *j);
    if (!j->contains("groups") && j->contains("tools")) {
        nlohmann::json groups = nlohmann::json::array();
        std::map<int, nlohmann::json> byGroup;
        for (const auto& t : (*j)["tools"]) {
            const int g = t.value("group", 0);
            auto& entry = byGroup[g];
            if (!entry.contains("group")) {
                entry = {{"group", g}, {"name", "Group " + std::to_string(g)}, {"ok", 0}, {"total", 0}};
            }
            entry["total"] = entry["total"].get<int>() + 1;
            if (t.value("status", "") == "OK") {
                entry["ok"] = entry["ok"].get<int>() + 1;
            }
        }
        for (auto& [_, g] : byGroup) {
            const int ok = g["ok"].get<int>();
            const int total = g["total"].get<int>();
            g["health_pct"] = total > 0 ? 100.0f * ok / total : 0.0f;
            groups.push_back(std::move(g));
        }
        s.raw["groups"] = groups;
    }
    return s;
}

BridgeClient::JobResult BridgeClient::PostJson(
    const std::string& path,
    const nlohmann::json& body,
    uint16_t port) {
    JobResult out;
    auto cli = MakeClient(port);
    auto res = cli.Post(path.c_str(), body.dump(), "application/json");
    if (!res) {
        out.message = "Bridge unreachable (start: uvicorn external_engine_bridge:app --port 8003)";
        return out;
    }
    try {
        out.raw = nlohmann::json::parse(res->body);
    } catch (...) {
        out.message = "Invalid JSON from bridge";
        return out;
    }
    if (res->status >= 400) {
        out.message = out.raw.value("detail", res->body);
        return out;
    }
    out.success = out.raw.value("status", "") == "success";
    if (!out.success && res->status == 200 && !out.raw.contains("detail")) {
        out.success = true;
    }
    out.backend = out.raw.value("backend", out.raw.value("engine", out.raw.value("engine_used", "")));
    out.filePath = out.raw.value(
        "step_file",
        out.raw.value("file_path", out.raw.value("mesh_path", out.raw.value("mesh_file", out.raw.value("output_path", "")))));
    if (out.message.empty() && !out.success) {
        out.message = out.raw.value("message", "Request failed");
    }
    return out;
}

BridgeClient::CadJobResult BridgeClient::TextToCad(
    const std::string& prompt,
    const std::string& backend,
    const std::string& outputDir,
    uint16_t port) {
    nlohmann::json body = {
        {"prompt", prompt},
        {"output_dir", outputDir},
        {"backend", backend},
    };
    auto result = PostJson("/cad/text-to-cad", body, port);
    if (result.success && result.filePath.empty()) {
        result.message = result.raw.value("engine", "CAD spec generated (no STEP file)");
    }
    return result;
}

BridgeClient::CadJobResult BridgeClient::ExportCad(
    const std::string& shape,
    float length,
    float width,
    float height,
    const std::string& backend,
    const std::string& outputPath,
    uint16_t port) {
    nlohmann::json body = {
        {"shape", shape},
        {"length", length},
        {"width", width},
        {"height", height},
        {"backend", backend},
        {"output_path", outputPath},
        {"format", "STEP"},
        {"model_data", ""},
    };
    return PostJson("/cad/export", body, port);
}

BridgeClient::CadJobResult BridgeClient::FitGeometry(
    const std::string& fitType,
    const nlohmann::json& points,
    uint16_t port) {
    nlohmann::json body = {
        {"fit_type", fitType},
        {"points", points},
    };
    return PostJson("/cad/fit", body, port);
}

BridgeClient::MeshJobResult BridgeClient::GenerateMesh(
    float lengthMm,
    float widthMm,
    float heightMm,
    float maxElementSizeM,
    const std::string& geometryPath,
    uint16_t port) {
    nlohmann::json body = {
        {"box_length", lengthMm / 1000.f},
        {"box_width", widthMm / 1000.f},
        {"box_height", heightMm / 1000.f},
        {"max_element_size", maxElementSizeM},
    };
    if (!geometryPath.empty()) {
        body["geometry_path"] = geometryPath;
    }
    return PostJson("/mesh/generate", body, port);
}

BridgeClient::MeshJobResult BridgeClient::TessellateMesh(
    const std::string& primitive,
    float lengthMm,
    float widthMm,
    float heightMm,
    float maxChordalError,
    bool includeMesh,
    uint16_t port) {
    nlohmann::json body = {
        {"primitive", primitive},
        {"length", lengthMm / 1000.f},
        {"width", widthMm / 1000.f},
        {"height", heightMm / 1000.f},
        {"max_chordal_error", maxChordalError},
        {"include_mesh", includeMesh},
        {"dynamic_lod", true},
    };
    return PostJson("/cad/mesh/tessellate", body, port);
}

BridgeClient::FeaJobResult BridgeClient::RunFea(
    int componentId,
    float forceN,
    float areaM2,
    float thicknessM,
    float youngsModulusPa,
    float yieldStrengthPa,
    float densityKgM3,
    uint16_t port) {
    nlohmann::json body = {
        {"component_id", componentId},
        {"force_n", forceN},
        {"area_m2", areaM2},
        {"thickness_m", thicknessM},
        {"youngs_modulus_pa", youngsModulusPa},
        {"yield_strength_pa", yieldStrengthPa},
        {"density_kg_m3", densityKgM3},
    };
    return PostJson("/simulation/fea/run", body, port);
}

void BridgeClient::ApplyReadyFlags(StackStatus& s, const nlohmann::json& j) {
    auto flag = [&](const char* key, bool& out) {
        if (j.contains(key)) {
            out = j.value(key, false);
        }
    };
    flag("cad_ready", s.cadReady);
    flag("mesh_ready", s.meshReady);
    flag("fea_ready", s.feaReady);
    flag("calculix_available", s.calculixAvailable);
    flag("thermal_ready", s.thermalReady);
    flag("cfd_ready", s.cfdReady);
    flag("optimization_ready", s.optimizationReady);
    flag("materials_ready", s.materialsReady);
    flag("manufacturing_ready", s.manufacturingReady);
    flag("robotics_ready", s.roboticsReady);
    flag("iot_ready", s.iotReady);
    flag("ml_ready", s.mlReady);
    flag("pipeline_ready", s.pipelineReady);
}

BridgeClient::JobResult BridgeClient::RunThermal(
    int componentId, const std::string& mode, float ambientTempC,
    float heatInputW, float convectionCoeff, uint16_t port) {
    nlohmann::json body = {
        {"component_id", componentId},
        {"mode", mode},
        {"ambient_temp_c", ambientTempC},
        {"heat_input_w", heatInputW},
        {"convection_coefficient", convectionCoeff},
        {"material_properties", nlohmann::json::object()},
    };
    return PostJson("/simulation/thermal/run", body, port);
}

BridgeClient::JobResult BridgeClient::RunCfd(
    int componentId, float inletVelocityMs, float fluidDensity, uint16_t port) {
    nlohmann::json body = {
        {"component_id", componentId},
        {"inlet_velocity_m_s", inletVelocityMs},
        {"fluid_density_kg_m3", fluidDensity > 0 ? fluidDensity : 1.225},
        {"bounding_box", {{"length", 1.0}, {"width", 0.5}, {"height", 0.5}}},
    };
    return PostJson("/simulation/cfd/run", body, port);
}

BridgeClient::JobResult BridgeClient::GetFluidProperties(
    const std::string& fluid, float tempK, uint16_t port) {
    const std::string path = "/simulation/thermal/fluid?fluid=" + fluid + "&T_k=" + std::to_string(tempK);
    JobResult out;
    auto j = GetJson(path, port);
    if (!j) {
        out.message = "Fluid lookup failed";
        return out;
    }
    out.raw = *j;
    out.success = j->value("status", "") == "success";
    out.backend = "CoolProp";
    return out;
}

BridgeClient::JobResult BridgeClient::RunOptimization(
    float initialStressMpa, float initialMassKg, float targetStressMpa, int iterations, uint16_t port) {
    nlohmann::json body = {
        {"initial_stress_mpa", initialStressMpa},
        {"initial_mass_kg", initialMassKg},
        {"target_stress_mpa", targetStressMpa},
        {"iterations", iterations},
    };
    return PostJson("/simulation/optimize", body, port);
}

BridgeClient::JobResult BridgeClient::LookupMaterial(const std::string& formula, uint16_t port) {
    JobResult out;
    auto j = GetJson("/materials/lookup/" + formula, port);
    if (!j) {
        out.message = "Material lookup failed";
        return out;
    }
    out.raw = *j;
    out.success = true;
    return out;
}

BridgeClient::JobResult BridgeClient::SearchNistMaterial(const std::string& material, uint16_t port) {
    JobResult out;
    auto j = GetJson("/data/search/nist?material=" + material, port);
    if (!j) {
        out.message = "NIST search failed";
        return out;
    }
    out.raw = *j;
    out.success = true;
    return out;
}

BridgeClient::JobResult BridgeClient::RunSlicer(
    int componentId, const std::string& name, float layerHeightMm, uint16_t port) {
    nlohmann::json body = {
        {"component_id", componentId},
        {"component_name", name},
        {"layer_height_mm", layerHeightMm},
        {"bounding_box", {{"length", 0.1}, {"width", 0.1}, {"height", 0.1}}},
    };
    return PostJson("/manufacturing/slicer/run", body, port);
}

BridgeClient::JobResult BridgeClient::ParseUrdf(const std::string& content, uint16_t port) {
    return PostJson("/robotics/urdf/parse", {{"content", content}}, port);
}

BridgeClient::JobResult BridgeClient::SimulateRobot(int steps, uint16_t port) {
    return PostJson("/robotics/simulate", {{"steps", steps}}, port);
}

BridgeClient::JobResult BridgeClient::StartMqtt(
    const std::string& broker, int port, const std::string& topic, uint16_t portNum) {
    nlohmann::json body = {{"broker", broker}, {"port", port}, {"topic", topic}};
    return PostJson("/iot/mqtt/start", body, portNum);
}

BridgeClient::JobResult BridgeClient::TrainSurrogate(const std::string& name, int epochs, uint16_t port) {
    return PostJson("/simulation/surrogate/train", {{"name", name}, {"epochs", epochs}}, port);
}

BridgeClient::JobResult BridgeClient::PredictSurrogate(
    const std::string& name, const nlohmann::json& inputs, uint16_t port) {
    return PostJson("/simulation/surrogate/predict", {{"name", name}, {"inputs", inputs}}, port);
}

BridgeClient::JobResult BridgeClient::RunPipeline(const std::string& geometryPath, uint16_t port) {
    return PostJson("/pipeline/run", {{"geometry_path", geometryPath}}, port);
}

BridgeClient::JobResult BridgeClient::GetPipelineJob(const std::string& jobId, uint16_t port) {
    JobResult out;
    auto j = GetJson("/pipeline/job/" + jobId, port);
    if (!j) {
        out.message = "Pipeline job not found";
        return out;
    }
    out.raw = *j;
    out.success = j->value("status", "") == "completed";
    return out;
}

} // namespace SZM
