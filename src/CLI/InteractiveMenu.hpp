#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace SZM::CLI {

class InteractiveMenu {
public:
    InteractiveMenu();
    
    void Run();
    void DisplayMainMenu();
    void DisplayScenarioMenu();
    void DisplaySettingsMenu();
    void DisplayHelpMenu();
    
private:
    void ShowWelcome();
    void ShowGoodbye();
    void ClearScreen();
    void PrintSeparator(char ch = '=', int width = 60);
    void PrintCentered(const std::string& text);
    
    int GetUserChoice(int maxOptions);
    std::string GetUserInput(const std::string& prompt);
    
    void RunQuickDemo();
    void RunTableDemo();
    void RunCustomScenario();
    void RunNormalOperation();
    void RunMechanicalOverload();
    void RunThermalOverload();
    
    void AdjustSimulationSpeed();
    void AdjustOutputVerbosity();
    void ResetToDefaults();
    
    void ShowAbout();
    void ShowControls();
    void ShowMaterialDatabase();
    
    // Configuration
    struct Config {
        double simulationSpeed = 1.0;
        bool verboseOutput = false;
        bool colorOutput = true;
        int outputPrecision = 2;
    } m_Config;
    
    bool m_Running = true;
};

} // namespace SZM::CLI
