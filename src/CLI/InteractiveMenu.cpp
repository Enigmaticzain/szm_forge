#include "InteractiveMenu.hpp"
#include "Simulation/SimulationEngine.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <limits>

namespace SZM::CLI {

namespace {
    constexpr float kAmbientTempK = 293.15f;
    constexpr float kG = 9.81f;

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

    std::string ColorCode(const std::string& status) {
        if (status == "SAFE") return "\033[32m";      // Green
        if (status == "WARN") return "\033[33m";      // Yellow
        if (status == "FAIL") return "\033[31m";      // Red
        return "\033[0m";                              // Reset
    }

    std::string ResetColor() {
        return "\033[0m";
    }

    void PrintComponentState(double simTimeSec, const SZM::SimulationComponent& comp, bool useColor) {
        std::string status = StatusLabel(comp);
        std::string color = useColor ? ColorCode(status) : "";
        std::string reset = useColor ? ResetColor() : "";

        std::cout << std::fixed << std::setprecision(2)
                  << "t=" << std::setw(5) << simTimeSec << " s | "
                  << std::left << std::setw(18) << comp.name << std::right
                  << " | Stress " << std::setw(7) << (comp.stress / 1e6f) << " MPa"
                  << " | Temp " << std::setw(7) << KelvinToCelsius(comp.temperature) << " C"
                  << " | Util " << std::setw(5) << (comp.stressRatio * 100.0f) << "% "
                  << color << status << reset << '\n';
    }

    void ConfigureComponent(SZM::SimulationComponent* comp, float areaM2, float densityKgPerM3, float yieldStrengthPa) {
        if (!comp) throw std::runtime_error("Null component");
        comp->area = areaM2;
        comp->density = densityKgPerM3;
        comp->yieldStrength = yieldStrengthPa;
        comp->temperature = kAmbientTempK;
    }
}

InteractiveMenu::InteractiveMenu() = default;

void InteractiveMenu::ClearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void InteractiveMenu::PrintSeparator(char ch, int width) {
    std::cout << std::string(width, ch) << '\n';
}

void InteractiveMenu::PrintCentered(const std::string& text) {
    int padding = (60 - text.length()) / 2;
    std::cout << std::string(padding, ' ') << text << '\n';
}

void InteractiveMenu::ShowWelcome() {
    ClearScreen();
    PrintSeparator('=', 60);
    PrintCentered("SZM FORGE v0.2.0");
    PrintCentered("Physics-Driven Engineering Platform");
    PrintSeparator('=', 60);
    std::cout << "\nWelcome! This interactive demo shows mechanical and thermal\n"
              << "simulation of machine parts under various load conditions.\n\n";
}

void InteractiveMenu::ShowGoodbye() {
    std::cout << "\n";
    PrintSeparator('=', 60);
    PrintCentered("Thank you for using SZM Forge!");
    PrintSeparator('=', 60);
    std::cout << '\n';
}

int InteractiveMenu::GetUserChoice(int maxOptions) {
    int choice = -1;
    while (choice < 1 || choice > maxOptions) {
        std::cout << "\nEnter choice (1-" << maxOptions << "): ";
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1;
        }
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

std::string InteractiveMenu::GetUserInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

void InteractiveMenu::Run() {
    ShowWelcome();
    
    while (m_Running) {
        DisplayMainMenu();
    }
    
    ShowGoodbye();
}

void InteractiveMenu::DisplayMainMenu() {
    std::cout << "\n--- MAIN MENU ---\n"
              << "1. Run Quick Demo (all scenarios)\n"
              << "2. Select Individual Scenario\n"
              << "3. Settings\n"
              << "4. Help & Documentation\n"
              << "5. Exit\n";
    
    int choice = GetUserChoice(5);
    
    switch (choice) {
        case 1: RunQuickDemo(); break;
        case 2: DisplayScenarioMenu(); break;
        case 3: DisplaySettingsMenu(); break;
        case 4: DisplayHelpMenu(); break;
        case 5: m_Running = false; break;
    }
}

void InteractiveMenu::DisplayScenarioMenu() {
    std::cout << "\n--- SELECT SCENARIO ---\n"
              << "1. Table Stress Analysis\n"
              << "2. Normal Operation (Beam + Bracket)\n"
              << "3. Mechanical Overload\n"
              << "4. Thermal Overload\n"
              << "5. Custom Scenario\n"
              << "6. Back to Main Menu\n";
    
    int choice = GetUserChoice(6);
    
    switch (choice) {
        case 1: RunTableDemo(); break;
        case 2: RunNormalOperation(); break;
        case 3: RunMechanicalOverload(); break;
        case 4: RunThermalOverload(); break;
        case 5: RunCustomScenario(); break;
        case 6: break;
    }
}

void InteractiveMenu::DisplaySettingsMenu() {
    std::cout << "\n--- SETTINGS ---\n"
              << "1. Simulation Speed (current: " << m_Config.simulationSpeed << "x)\n"
              << "2. Verbose Output (current: " << (m_Config.verboseOutput ? "ON" : "OFF") << ")\n"
              << "3. Color Output (current: " << (m_Config.colorOutput ? "ON" : "OFF") << ")\n"
              << "4. Reset to Defaults\n"
              << "5. Back to Main Menu\n";
    
    int choice = GetUserChoice(5);
    
    switch (choice) {
        case 1: AdjustSimulationSpeed(); break;
        case 2: m_Config.verboseOutput = !m_Config.verboseOutput; break;
        case 3: m_Config.colorOutput = !m_Config.colorOutput; break;
        case 4: ResetToDefaults(); break;
        case 5: break;
    }
}

void InteractiveMenu::DisplayHelpMenu() {
    std::cout << "\n--- HELP & DOCUMENTATION ---\n"
              << "1. About SZM Forge\n"
              << "2. How to Read Results\n"
              << "3. Material Database\n"
              << "4. Keyboard Controls\n"
              << "5. Back to Main Menu\n";
    
    int choice = GetUserChoice(5);
    
    switch (choice) {
        case 1: ShowAbout(); break;
        case 2: ShowControls(); break;
        case 3: ShowMaterialDatabase(); break;
        case 4: std::cout << "\nNo keyboard controls in CLI mode.\n"; break;
        case 5: break;
    }
}

void InteractiveMenu::RunQuickDemo() {
    std::cout << "\n[Running Quick Demo - All Scenarios]\n";
    RunTableDemo();
    RunNormalOperation();
    RunMechanicalOverload();
    RunThermalOverload();
}

void InteractiveMenu::RunTableDemo() {
    std::cout << "\n" << std::string(60, '=') << '\n';
    std::cout << "TABLE DEMO — Steel table with 200 kg load\n";
    std::cout << "Parts: 1 tabletop + 4 legs | Material: Structural Steel\n";
    std::cout << std::string(60, '=') << '\n';

    SZM::SimulationEngine& engine = SZM::SimulationEngine::GetInstance();
    engine.Init();

    constexpr float kLoadKg = 200.0f;
    constexpr float kTotalForce = kLoadKg * kG;
    constexpr float kLegForce = kTotalForce / 4.0f;
    constexpr float kTopL = 0.6f, kTopB = 0.8f, kTopT = 0.025f;
    constexpr float kTopEffArea = (4.0f * kTopB * kTopT * kTopT) / (3.0f * kTopL * kTopL);
    constexpr float kLegArea = 0.05f * 0.05f;

    const uint32_t topId = engine.AddComponent("Tabletop", "MAT-STEEL-STRUCT", kTopEffArea, kTopT);
    const uint32_t l1Id = engine.AddComponent("Leg-FL", "MAT-STEEL-STRUCT", kLegArea, 0.75f);
    const uint32_t l2Id = engine.AddComponent("Leg-FR", "MAT-STEEL-STRUCT", kLegArea, 0.75f);
    const uint32_t l3Id = engine.AddComponent("Leg-RL", "MAT-STEEL-STRUCT", kLegArea, 0.75f);
    const uint32_t l4Id = engine.AddComponent("Leg-RR", "MAT-STEEL-STRUCT", kLegArea, 0.75f);

    engine.SetForce(topId, kTotalForce);
    for (uint32_t legId : {l1Id, l2Id, l3Id, l4Id}) {
        engine.SetForce(legId, kLegForce);
    }

    engine.Tick(0.0);

    std::cout << "\nLoad: " << kLoadKg << " kg (" << kTotalForce << " N total)\n\n";
    std::cout << std::left << std::setw(12) << "Part"
              << std::right << std::setw(14) << "Stress (MPa)"
              << std::setw(14) << "Yield (MPa)"
              << std::setw(14) << "Utilization"
              << std::setw(10) << "Status\n"
              << std::string(64, '-') << '\n';

    for (uint32_t pid : {topId, l1Id, l2Id, l3Id, l4Id}) {
        const auto* p = engine.GetComponent(pid);
        if (!p) continue;
        std::string status = (p->stressRatio >= 1.0f) ? "FAIL" : (p->isDangerous) ? "WARN" : "SAFE";
        std::cout << std::fixed << std::setprecision(2)
                  << std::left << std::setw(12) << p->name
                  << std::right << std::setw(14) << (p->stress / 1e6f)
                  << std::setw(14) << (p->yieldStrength / 1e6f)
                  << std::setw(13) << (p->stressRatio * 100.0f) << "%"
                  << std::setw(10) << status << '\n';
    }

    std::cout << "\nTable demo complete.\n";
    engine.Shutdown();
}

void InteractiveMenu::RunNormalOperation() {
    std::cout << "\n" << std::string(60, '=') << '\n';
    std::cout << "SCENARIO 1: Normal Operation\n";
    std::cout << "Safe load and mild heating. Both parts should stay stable.\n";
    std::cout << std::string(60, '=') << '\n';

    SZM::SimulationEngine& engine = SZM::SimulationEngine::GetInstance();
    engine.Init();

    const uint32_t beamId = engine.AddComponent("Steel Beam");
    const uint32_t bracketId = engine.AddComponent("Al Bracket");

    ConfigureComponent(engine.GetComponent(beamId), 0.0050f, 7850.0f, 250e6f);
    ConfigureComponent(engine.GetComponent(bracketId), 0.0008f, 2700.0f, 95e6f);

    engine.SetForce(beamId, 150000.0f);
    engine.SetHeatInput(beamId, 0.8f);
    engine.SetForce(bracketId, 22000.0f);
    engine.SetHeatInput(bracketId, 0.0f);

    double simTime = 0.0;
    for (int i = 0; i < 6; ++i) {
        engine.Tick(0.5);
        simTime += 0.5;
        if (auto* beam = engine.GetComponent(beamId)) {
            PrintComponentState(simTime, *beam, m_Config.colorOutput);
        }
        if (auto* bracket = engine.GetComponent(bracketId)) {
            PrintComponentState(simTime, *bracket, m_Config.colorOutput);
        }
    }

    engine.Shutdown();
}

void InteractiveMenu::RunMechanicalOverload() {
    std::cout << "\n" << std::string(60, '=') << '\n';
    std::cout << "SCENARIO 2: Mechanical Overload\n";
    std::cout << "Force spike on aluminum bracket to demonstrate stress failure.\n";
    std::cout << std::string(60, '=') << '\n';

    SZM::SimulationEngine& engine = SZM::SimulationEngine::GetInstance();
    engine.Init();

    const uint32_t beamId = engine.AddComponent("Steel Beam");
    const uint32_t bracketId = engine.AddComponent("Al Bracket");

    ConfigureComponent(engine.GetComponent(beamId), 0.0050f, 7850.0f, 250e6f);
    ConfigureComponent(engine.GetComponent(bracketId), 0.0008f, 2700.0f, 95e6f);

    engine.SetForce(beamId, 160000.0f);
    engine.SetHeatInput(beamId, 0.8f);
    engine.SetForce(bracketId, 95000.0f);
    engine.SetHeatInput(bracketId, 0.0f);

    double simTime = 0.0;
    for (int i = 0; i < 6; ++i) {
        engine.Tick(0.5);
        simTime += 0.5;
        if (auto* beam = engine.GetComponent(beamId)) {
            PrintComponentState(simTime, *beam, m_Config.colorOutput);
        }
        if (auto* bracket = engine.GetComponent(bracketId)) {
            PrintComponentState(simTime, *bracket, m_Config.colorOutput);
        }
    }

    engine.Shutdown();
}

void InteractiveMenu::RunThermalOverload() {
    std::cout << "\n" << std::string(60, '=') << '\n';
    std::cout << "SCENARIO 3: Thermal Overload\n";
    std::cout << "Heat input spike on steel beam to show thermal danger.\n";
    std::cout << std::string(60, '=') << '\n';

    SZM::SimulationEngine& engine = SZM::SimulationEngine::GetInstance();
    engine.Init();

    const uint32_t beamId = engine.AddComponent("Steel Beam");
    const uint32_t bracketId = engine.AddComponent("Al Bracket");

    ConfigureComponent(engine.GetComponent(beamId), 0.0050f, 7850.0f, 250e6f);
    ConfigureComponent(engine.GetComponent(bracketId), 0.0008f, 2700.0f, 95e6f);

    engine.SetForce(beamId, 160000.0f);
    engine.SetHeatInput(beamId, 10.0f);
    engine.SetForce(bracketId, 30000.0f);
    engine.SetHeatInput(bracketId, 0.0f);

    double simTime = 0.0;
    for (int i = 0; i < 10; ++i) {
        engine.Tick(0.5);
        simTime += 0.5;
        if (auto* beam = engine.GetComponent(beamId)) {
            PrintComponentState(simTime, *beam, m_Config.colorOutput);
        }
        if (auto* bracket = engine.GetComponent(bracketId)) {
            PrintComponentState(simTime, *bracket, m_Config.colorOutput);
        }
    }

    engine.Shutdown();
}

void InteractiveMenu::RunCustomScenario() {
    std::cout << "\n[Custom Scenario - Not yet implemented]\n"
              << "This feature will allow you to set custom forces and heat inputs.\n";
}

void InteractiveMenu::AdjustSimulationSpeed() {
    std::cout << "\nCurrent simulation speed: " << m_Config.simulationSpeed << "x\n"
              << "Enter new speed (0.5 = half speed, 2.0 = double speed): ";
    std::cin >> m_Config.simulationSpeed;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Speed set to " << m_Config.simulationSpeed << "x\n";
}

void InteractiveMenu::ResetToDefaults() {
    m_Config.simulationSpeed = 1.0;
    m_Config.verboseOutput = false;
    m_Config.colorOutput = true;
    m_Config.outputPrecision = 2;
    std::cout << "\nSettings reset to defaults.\n";
}

void InteractiveMenu::ShowAbout() {
    std::cout << "\n--- ABOUT SZM FORGE ---\n"
              << "SZM Forge is a physics-driven engineering platform that simulates\n"
              << "mechanical and thermal behavior of machine parts under load.\n\n"
              << "Version: 0.2.0\n"
              << "Status: Phase 1 Demo Ready\n"
              << "Language: C++20\n\n"
              << "Features:\n"
              << "  • Stress analysis (Von Mises equivalent stress)\n"
              << "  • Thermal analysis (transient heat diffusion)\n"
              << "  • Material database with real properties\n"
              << "  • Multi-scenario demonstration\n"
              << "  • Interactive CLI interface\n";
}

void InteractiveMenu::ShowControls() {
    std::cout << "\n--- HOW TO READ RESULTS ---\n"
              << "Each simulation step shows:\n\n"
              << "  t=X.XX s          : Simulation time in seconds\n"
              << "  Stress X.XX MPa   : Mechanical load per unit area\n"
              << "  Temp X.XX C       : Component temperature in Celsius\n"
              << "  Util X.XX%        : Utilization ratio (0-100%)\n"
              << "  Status            : SAFE / WARN / FAIL\n\n"
              << "Status Codes:\n"
              << "  SAFE : All metrics < 70% utilization\n"
              << "  WARN : Any metric 70-99% utilization\n"
              << "  FAIL : Any metric >= 100% utilization\n";
}

void InteractiveMenu::ShowMaterialDatabase() {
    std::cout << "\n--- MATERIAL DATABASE ---\n"
              << "Steel (Structural):\n"
              << "  Yield Strength: 250 MPa\n"
              << "  Density: 7,850 kg/m³\n"
              << "  Thermal Limit: ~400°C\n\n"
              << "Aluminum (6061-T6):\n"
              << "  Yield Strength: 95 MPa\n"
              << "  Density: 2,700 kg/m³\n"
              << "  Thermal Limit: ~150°C\n";
}

} // namespace SZM::CLI
