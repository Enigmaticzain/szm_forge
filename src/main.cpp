/**
 * SZM Forge — Unified Entry Point
 *
 * Single entry point supporting multiple modes:
 * - Lightweight mode (default): Terminal-based simulation demo
 * - Desktop mode: Full GUI with ImGui dockspace
 * - Interactive mode: Menu-driven CLI
 * - Code8 pipeline: FEA tool
 *
 * Build with:
 *   Default (lightweight): cmake -B build -DCMAKE_BUILD_TYPE=Release
 *   Desktop: cmake -B build -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
 */

#include <iostream>
#include <iomanip>
#include <exception>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

// Mode selection via preprocessor
#if defined(SZM_MODE_DESKTOP)
    #include "Core/Application.hpp"
    #include "Simulation/SimulationEngine.hpp"
    #include "UI/UIManager.hpp"
    #include "UI/panels/ConsolePanel.hpp"
    #include "UI/panels/SceneOutliner.hpp"
    #include "UI/panels/PropertiesPanel.hpp"
    #include "UI/panels/SimulationPanel.hpp"
    #include "UI/panels/SimulationVisualizerPanel.hpp"
    #include "UI/panels/ComponentBuilderPanel.hpp"
    #include "UI/panels/ToolbarPanel.hpp"
    #include "UI/panels/DashboardPanel.hpp"
    #include "Workshop3D/WorkshopManager.hpp"
    #include "Workshop3D/ComponentBrowserPanel.hpp"
    #include "Workshop3D/ComponentWeightTablePanel.hpp"
    #include "Workshop3D/WorkshopPanels.hpp"
    #include "WebViewer/ReactUIManager.hpp"
#elif defined(SZM_MODE_INTERACTIVE)
    #include "CLI/InteractiveMenu.hpp"
#else
    // Lightweight mode - simulation only
    #include "Simulation/SimulationEngine.hpp"
#endif

#include "DigitalTwin/DigitalTwinManager.hpp"
#include "DigitalTwin/SensorTelemetry.hpp"
#include "Geometry/operations/LoftOp.hpp"
#include "Geometry/TopologyValidator.hpp"
#include "Assembly/AssemblyGraph.hpp"
#include "AI/AIEngine.hpp"
#include "AI/AIOperatingSystem.hpp"
#include "AI/Predictions/FailurePredictor.hpp"
#include "WebAPI/FastAPIBridge.hpp"

namespace {

constexpr float kAmbientTempK = 293.15f;

float KelvinToCelsius(float kelvin) {
    return kelvin - 273.15f;
}

std::string StatusLabel(const SZM::SimulationComponent& comp) {
    if (comp.stressRatio >= 1.0f || comp.tempRatio >= 1.0f) {
        return "FAIL";
    }
    if (comp.isDangerous) {
        return "WARN";
    }
    return "SAFE";
}

void ConfigureComponent(
    SZM::SimulationComponent* comp,
    float areaM2,
    float densityKgPerM3,
    float yieldStrengthPa
) {
    if (!comp) {
        throw std::runtime_error("Null simulation component during setup.");
    }

    comp->area = areaM2;
    comp->density = densityKgPerM3;
    comp->yieldStrength = yieldStrengthPa;
    comp->temperature = kAmbientTempK;
}

void PrintHeader() {
    std::cout
        << "\n============================================================\n"
        << " SZM Forge - Simple Mechanical Demo\n"
        << " Topic: stress + thermal safety of two machine parts\n"
        << "============================================================\n";
}

void RunTableDemo(SZM::SimulationEngine& engine) {
    std::cout
        << "\n============================================================\n"
        << " TABLE DEMO — Steel table with 200 kg load\n"
        << " Parts: 1 tabletop + 4 legs | Material: Structural Steel\n"
        << "============================================================\n";

    constexpr float kG          = 9.81f;
    constexpr float kLoadKg     = 200.0f;
    constexpr float kTotalForce = kLoadKg * kG;
    constexpr float kLegForce   = kTotalForce / 4.0f;

    constexpr float kTopL = 0.6f, kTopB = 0.8f, kTopT = 0.025f;
    constexpr float kTopEffArea = (4.0f * kTopB * kTopT * kTopT) / (3.0f * kTopL * kTopL);
    constexpr float kLegArea = 0.05f * 0.05f;

    const uint32_t topId = engine.AddComponent("Tabletop",  "MAT-STEEL-STRUCT", kTopEffArea,  kTopT);
    const uint32_t l1Id  = engine.AddComponent("Leg-FL",    "MAT-STEEL-STRUCT", kLegArea,     0.75f);
    const uint32_t l2Id  = engine.AddComponent("Leg-FR",    "MAT-STEEL-STRUCT", kLegArea,     0.75f);
    const uint32_t l3Id  = engine.AddComponent("Leg-RL",    "MAT-STEEL-STRUCT", kLegArea,     0.75f);
    const uint32_t l4Id  = engine.AddComponent("Leg-RR",    "MAT-STEEL-STRUCT", kLegArea,     0.75f);

    engine.SetForce(topId, kTotalForce);
    for (uint32_t legId : {l1Id, l2Id, l3Id, l4Id}) {
        engine.SetForce(legId, kLegForce);
    }

    engine.Tick(0.0);

    std::cout << "\nLoad: " << kLoadKg << " kg  (" << kTotalForce << " N total)\n\n";
    std::cout << std::left
              << std::setw(12) << "Part"
              << std::right
              << std::setw(14) << "Stress (MPa)"
              << std::setw(14) << "Yield (MPa)"
              << std::setw(14) << "Utilization"
              << std::setw(10) << "Status"
              << '\n'
              << std::string(64, '-') << '\n';

    const std::vector<uint32_t> allIds = {topId, l1Id, l2Id, l3Id, l4Id};
    for (uint32_t pid : allIds) {
        const auto* p = engine.GetComponent(pid);
        if (!p) { continue; }
        std::string status = (p->stressRatio >= 1.0f) ? "FAIL"
                           : (p->isDangerous)          ? "WARN"
                                                       : "SAFE";
        std::cout << std::fixed << std::setprecision(2)
                  << std::left  << std::setw(12) << p->name
                  << std::right
                  << std::setw(14) << (p->stress / 1e6f)
                  << std::setw(14) << (p->yieldStrength / 1e6f)
                  << std::setw(13) << (p->stressRatio * 100.0f) << "%"
                  << std::setw(10) << status
                  << '\n';
    }

    std::cout << "\n--- Overload: +500 kg extra weight ---\n";
    constexpr float kOverloadForce = (kLoadKg + 500.0f) * kG;
    engine.SetForce(topId, kOverloadForce);
    for (uint32_t legId : {l1Id, l2Id, l3Id, l4Id}) {
        engine.SetForce(legId, kOverloadForce / 4.0f);
    }
    engine.Tick(0.0);

    std::cout << std::string(64, '-') << '\n';
    for (uint32_t pid : allIds) {
        const auto* p = engine.GetComponent(pid);
        if (!p) { continue; }
        std::string status = (p->stressRatio >= 1.0f) ? "FAIL"
                           : (p->isDangerous)          ? "WARN"
                                                       : "SAFE";
        std::cout << std::fixed << std::setprecision(2)
                  << std::left  << std::setw(12) << p->name
                  << std::right
                  << std::setw(14) << (p->stress / 1e6f)
                  << std::setw(14) << (p->yieldStrength / 1e6f)
                  << std::setw(13) << (p->stressRatio * 100.0f) << "%"
                  << std::setw(10) << status
                  << '\n';
    }

    std::cout << "\nTable demo complete.\n";
}

void PrintComponentState(double simTimeSec, const SZM::SimulationComponent& comp) {
    std::cout << std::fixed << std::setprecision(2)
              << "t=" << std::setw(5) << simTimeSec << " s | "
              << std::left << std::setw(18) << comp.name << std::right
              << " | Stress " << std::setw(7) << (comp.stress / 1e6f) << " MPa"
              << " | Temp " << std::setw(7) << KelvinToCelsius(comp.temperature) << " C"
              << " | Utilization "
              << std::setw(6) << (comp.stressRatio * 100.0f) << "% stress, "
              << std::setw(6) << (comp.tempRatio * 100.0f) << "% temp"
              << " | " << StatusLabel(comp) << '\n';
}

void RunScenario(
    const std::string& title,
    const std::string& note,
    SZM::SimulationEngine& engine,
    std::uint32_t beamId,
    std::uint32_t bracketId,
    float beamForceN,
    float beamHeatW,
    float bracketForceN,
    float bracketHeatW,
    int steps,
    double dt,
    double& simTimeSec
) {
    std::cout << "\n--- " << title << " ---\n" << note << '\n';

    engine.SetForce(beamId, beamForceN);
    engine.SetHeatInput(beamId, beamHeatW);
    engine.SetForce(bracketId, bracketForceN);
    engine.SetHeatInput(bracketId, bracketHeatW);

    for (int i = 0; i < steps; ++i) {
        engine.Tick(dt);
        simTimeSec += dt;

        if (auto* beam = engine.GetComponent(beamId)) {
            PrintComponentState(simTimeSec, *beam);
            // Push to WebAPI
            SZM::WebAPI::APIManager::GetInstance().PushComponentSnapshot(
                beam->name, beam->stress, beam->temperature,
                beam->stressRatio, beam->tempRatio, simTimeSec);
            // AI Analysis
            SZM::AI::DesignMetrics metrics;
            metrics.stress = beam->stressRatio;
            metrics.temperature = beam->temperature;
            if (beam->stressRatio > 0.8f) {
                auto pred = SZM::AI::AIEngine::GetInstance().PredictFailure(beamId, metrics);
                std::cout << "  \033[31m[AI ALERT]\033[0m " << pred.failureMode 
                          << " (Prob: " << (pred.probability*100) << "%)\n";
                auto opts = SZM::AI::Predictions::PredictionManager::GetInstance().GetOptimizer().SuggestOptimizations(beamId, 1);
                if (!opts.empty()) {
                    std::cout << "  \033[33m[AI SUGGEST]\033[0m " << opts[0].description << "\n";
                }
            }
        }
        if (auto* bracket = engine.GetComponent(bracketId)) {
            PrintComponentState(simTimeSec, *bracket);
            // Push to WebAPI
            SZM::WebAPI::APIManager::GetInstance().PushComponentSnapshot(
                bracket->name, bracket->stress, bracket->temperature,
                bracket->stressRatio, bracket->tempRatio, simTimeSec);
            // AI Analysis
            SZM::AI::DesignMetrics metrics;
            metrics.stress = bracket->stressRatio;
            metrics.temperature = bracket->temperature;
            if (bracket->tempRatio > 0.8f) {
                auto pred = SZM::AI::AIEngine::GetInstance().PredictFailure(bracketId, metrics);
                std::cout << "  \033[31m[AI ALERT]\033[0m " << pred.failureMode 
                          << " (Prob: " << (pred.probability*100) << "%)\n";
                auto opts = SZM::AI::Predictions::PredictionManager::GetInstance().GetOptimizer().SuggestOptimizations(bracketId, 1);
                if (!opts.empty()) {
                    std::cout << "  \033[33m[AI SUGGEST]\033[0m " << opts[0].description << "\n";
                }
            }
        }
        std::cout << '\n';
    }
}

void RunPhase4Demo() {
    std::cout << "\n============================================================\n";
    std::cout << " PHASE 4 DEMO — Lofting & Hierarchy\n";
    std::cout << "============================================================\n";

    // 1. Test Lofting
    SZM::Geometry::SketchProfile bottom, top;
    
    // Bottom square (outer loop)
    bottom.Loops.push_back({ {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0} });
    // Top square (smaller outer loop)
    top.Loops.push_back({ {0.2,0.2,0}, {0.8,0.2,0}, {0.8,0.8,0}, {0.2,0.8,0} });

    auto loftBody = SZM::Geometry::Operations::LoftOp::LoftProfiles({bottom, top});
    auto report = SZM::Geometry::TopologyValidator::Validate(loftBody);
    std::cout << "[Loft] Generated lofted body.\n";
    std::cout << "[Loft] Vertices: " << loftBody.GetVertexCount() 
              << ", Faces: " << loftBody.GetFaceCount() << "\n";
    std::cout << "[Loft] Topology Valid (Manifold): " << (report.isManifold ? "Yes" : "No") << "\n";

    // 2. Test Hierarchy
    auto root = std::make_shared<SZM::Entity>("Root");
    SZM::Assembly::AssemblyGraphManager asmManager(root);

    auto enginePart = std::make_shared<SZM::Assembly::BasePart>();
    enginePart->PartID = SZM::UUID{1,0};
    enginePart->PartName = "V8 Engine Block";
    enginePart->Mass = 150.0f;
    asmManager.RegisterBasePart(enginePart);

    auto pistonPart = std::make_shared<SZM::Assembly::BasePart>();
    pistonPart->PartID = SZM::UUID{2,0};
    pistonPart->PartName = "Piston";
    pistonPart->Mass = 0.5f;
    asmManager.RegisterBasePart(pistonPart);

    auto engineAssembly = std::make_shared<SZM::Assembly::AssemblyTemplate>();
    engineAssembly->TemplateID = SZM::UUID{3,0};
    engineAssembly->TemplateName = "Engine Assembly";
    
    // Add block
    engineAssembly->Components.push_back({enginePart->PartID, SZM::Assembly::AssetType::Part, SZM::Transform()});
    // Add 8 pistons
    for (int i=0; i<8; ++i) {
        engineAssembly->Components.push_back({pistonPart->PartID, SZM::Assembly::AssetType::Part, SZM::Transform()});
    }
    asmManager.RegisterAssemblyTemplate(engineAssembly);

    // Instantiate Assembly
    asmManager.InstantiateAsset(root, engineAssembly->TemplateID, SZM::Assembly::AssetType::Assembly, SZM::Transform(), "My Engine");

    auto bom = asmManager.GenerateBOM();
    std::cout << "\n[Hierarchy] BOM for 'My Engine':\n";
    for (const auto& entry : bom) {
        std::cout << " - " << entry.Quantity << "x " << entry.PartName 
                  << " (Total Mass: " << entry.TotalMass << " kg)\n";
    }

    float totalMass;
    float cg[3];
    asmManager.CalculateMassProperties(totalMass, cg);
    std::cout << "[Hierarchy] Total Assembly Mass: " << totalMass << " kg\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
#if defined(SZM_MODE_DESKTOP)
        // Desktop mode - Full GUI with React UI
        std::cout << "[SZM Forge] Starting desktop application with React UI...\n";

        SZM::WindowConfig windowConfig;
        windowConfig.Width = 1920;
        windowConfig.Height = 1080;
        windowConfig.Title = "SZM Forge — Physics-Driven Engineering Platform";
        windowConfig.VSync = true;

        SZM::Application& app = SZM::Application::GetInstance();
        app.Init(windowConfig);

        SZM::DigitalTwinManager::GetInstance().Initialize();
        SZM::SensorTelemetry::GetInstance().Initialize();

        std::cout << "[SZM Forge] Application initialized.\n";
        
        std::cout << "[SZM Forge] Entering main loop...\n\n";

        app.Run();

        // Shutdown React UI
        SZM::WebViewer::ReactUIManager::GetInstance().StopDevServer();

        SZM::DigitalTwinManager::GetInstance().Shutdown();
        SZM::SensorTelemetry::GetInstance().Shutdown();
        SZM::SimulationEngine::GetInstance().Shutdown();
        app.Shutdown();

        std::cout << "\n[SZM Forge] Application terminated successfully.\n";

#elif defined(SZM_MODE_INTERACTIVE)
        // Interactive mode - CLI menu
        SZM::CLI::InteractiveMenu menu;
        menu.Run();

#else
        // Lightweight mode - Terminal simulation
        std::cout << "[SZM Simulation] Engine initialized.\n";

        SZM::DigitalTwinManager::GetInstance().Initialize();
        SZM::SensorTelemetry::GetInstance().Initialize();
        SZM::AI::AIOperatingSystem::GetInstance().Initialize();
        SZM::AI::Predictions::PredictionManager::GetInstance().Initialize("models");
        SZM::WebAPI::APIManager::GetInstance().Initialize(8000);

        SZM::SimulationEngine& engine = SZM::SimulationEngine::GetInstance();
        engine.Init();

        const std::uint32_t beamId = engine.AddComponent("Steel Beam");
        const std::uint32_t bracketId = engine.AddComponent("Al Bracket");

        ConfigureComponent(engine.GetComponent(beamId), 0.0050f, 7850.0f, 250e6f);
        ConfigureComponent(engine.GetComponent(bracketId), 0.0008f, 2700.0f, 95e6f);

        RunTableDemo(engine);
        
        double simTimeSec = 0.0;
        RunScenario("Scenario 1: Normal Operating Load", 
                    "Applying 15,000 N to steel beam, 2,000 N to aluminum bracket.",
                    engine, beamId, bracketId, 
                    15000.0f, 0.0f,   // Beam force/heat
                     2000.0f, 0.0f,   // Bracket force/heat
                    6, 0.5, simTimeSec);

        RunScenario("Scenario 2: Structural Overload",
                    "Applying 1,500,000 N to steel beam to trigger AI warnings.",
                    engine, beamId, bracketId,
                    1500000.0f, 0.0f,
                    2000.0f, 0.0f,
                    6, 0.5, simTimeSec);

        RunScenario("Scenario 3: Thermal Overload",
                    "Heat input spike on aluminum bracket to show thermal danger.",
                    engine, beamId, bracketId,
                    15000.0f, 0.0f,
                    2000.0f, 5000.0f,
                    10, 0.5, simTimeSec);

        RunPhase4Demo();

        std::cout << "\n=== Final Snapshot ===\n";
        if (auto* beam = engine.GetComponent(beamId)) {
            PrintComponentState(simTimeSec, *beam);
        }
        if (auto* bracket = engine.GetComponent(bracketId)) {
            PrintComponentState(simTimeSec, *bracket);
        }

        std::cout << "\nSimulation completed successfully. Keeping API online for 15 seconds...\n";
        std::cout << "You can test it by running: curl http://localhost:8000/api/simulation/status\n";
        std::this_thread::sleep_for(std::chrono::seconds(15));

        engine.Shutdown();
        SZM::AI::Predictions::PredictionManager::GetInstance().Shutdown();
        SZM::AI::AIOperatingSystem::GetInstance().Shutdown();
        SZM::SensorTelemetry::GetInstance().Shutdown();
        SZM::DigitalTwinManager::GetInstance().Shutdown();
        SZM::WebAPI::APIManager::GetInstance().Shutdown();
#endif

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr << "[SZM FATAL] " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
