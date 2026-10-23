#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace SZM {

/** HTTP client for Python External Engine Bridge (127.0.0.1:8003). */
class BridgeClient {
public:
    static constexpr uint16_t DefaultPort = 8003;
    static constexpr const char* DefaultHost = "127.0.0.1";

    struct StackStatus {
        bool connected = false;
        bool cadReady = false;
        bool meshReady = false;
        bool feaReady = false;
        bool calculixAvailable = false;
        bool thermalReady = false;
        bool cfdReady = false;
        bool optimizationReady = false;
        bool materialsReady = false;
        bool manufacturingReady = false;
        bool roboticsReady = false;
        bool iotReady = false;
        bool mlReady = false;
        bool pipelineReady = false;
        int available = 0;
        int totalTools = 0;
        std::string status;
        std::string lastError;
        nlohmann::json raw;
    };

    struct JobResult {
        bool success = false;
        std::string message;
        std::string backend;
        std::string filePath;
        nlohmann::json raw;
    };

    using CadJobResult = JobResult;
    using MeshJobResult = JobResult;
    using FeaJobResult = JobResult;

    static bool IsReachable(uint16_t port = DefaultPort);
    static std::optional<StackStatus> FetchStackStatus(uint16_t port = DefaultPort);

    // Group 2 — CAD
    static JobResult TextToCad(const std::string& prompt, const std::string& backend = "auto",
        const std::string& outputDir = "/tmp", uint16_t port = DefaultPort);
    static JobResult ExportCad(const std::string& shape, float length, float width, float height,
        const std::string& backend = "auto", const std::string& outputPath = "/tmp/szm_export.step",
        uint16_t port = DefaultPort);
    static JobResult FitGeometry(const std::string& fitType, const nlohmann::json& points,
        uint16_t port = DefaultPort);

    // Group 3 — Mesh
    static JobResult GenerateMesh(float lengthMm, float widthMm, float heightMm,
        float maxElementSizeM = 0.05f, const std::string& geometryPath = "", uint16_t port = DefaultPort);
    static JobResult TessellateMesh(const std::string& primitive, float lengthMm, float widthMm,
        float heightMm, float maxChordalError = 0.01f, bool includeMesh = false, uint16_t port = DefaultPort);

    // Group 4 — FEA
    static JobResult RunFea(int componentId, float forceN, float areaM2, float thicknessM,
        float youngsModulusPa = 210e9f, float yieldStrengthPa = 250e6f, float densityKgM3 = 7850.f,
        uint16_t port = DefaultPort);

    // Group 5 — Thermal/CFD
    static JobResult RunThermal(int componentId, const std::string& mode, float ambientTempC,
        float heatInputW, float convectionCoeff = 10.f, uint16_t port = DefaultPort);
    static JobResult RunCfd(int componentId, float inletVelocityMs, float fluidDensity = 0.f,
        uint16_t port = DefaultPort);
    static JobResult GetFluidProperties(const std::string& fluid, float tempK = 300.f,
        uint16_t port = DefaultPort);

    // Group 7 — Optimization
    static JobResult RunOptimization(float initialStressMpa, float initialMassKg,
        float targetStressMpa, int iterations = 50, uint16_t port = DefaultPort);

    // Group 8 — Materials
    static JobResult LookupMaterial(const std::string& formula, uint16_t port = DefaultPort);
    static JobResult SearchNistMaterial(const std::string& material, uint16_t port = DefaultPort);

    // Group 11 — Manufacturing
    static JobResult RunSlicer(int componentId, const std::string& name, float layerHeightMm = 0.2f,
        uint16_t port = DefaultPort);

    // Group 10 — Robotics
    static JobResult ParseUrdf(const std::string& content, uint16_t port = DefaultPort);
    static JobResult SimulateRobot(int steps = 240, uint16_t port = DefaultPort);

    // Group 9 — IoT
    static JobResult StartMqtt(const std::string& broker = "localhost", int port = 1883,
        const std::string& topic = "szm/telemetry/#", uint16_t portNum = DefaultPort);

    // Group 6 — Physics ML
    static JobResult TrainSurrogate(const std::string& name = "default", int epochs = 100,
        uint16_t port = DefaultPort);
    static JobResult PredictSurrogate(const std::string& name, const nlohmann::json& inputs,
        uint16_t port = DefaultPort);

    // Group 15 — Pipeline
    static JobResult RunPipeline(const std::string& geometryPath, uint16_t port = DefaultPort);
    static JobResult GetPipelineJob(const std::string& jobId, uint16_t port = DefaultPort);

private:
    static std::optional<nlohmann::json> GetJson(const std::string& path, uint16_t port);
    static JobResult PostJson(const std::string& path, const nlohmann::json& body, uint16_t port);
    static void ApplyReadyFlags(StackStatus& s, const nlohmann::json& j);
};

} // namespace SZM
