#include "AIOperatingSystem.hpp"
#include <iostream>

namespace SZM::AI {

AIOperatingSystem& AIOperatingSystem::GetInstance() {
    static AIOperatingSystem instance;
    return instance;
}

void AIOperatingSystem::Initialize() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                                  ║\n";
    std::cout << "║         🤖 SZM FORGE AI OPERATING SYSTEM v1.0 🤖                ║\n";
    std::cout << "║                                                                  ║\n";
    std::cout << "║    Legendary AI Assistant - Autonomous Engineering Intelligence  ║\n";
    std::cout << "║                                                                  ║\n";
    std::cout << "║    Creator Mode: LOCKED                                          ║\n";
    std::cout << "║    AI Status: STANDBY                                            ║\n";
    std::cout << "║    Project Awareness: READY                                      ║\n";
    std::cout << "║                                                                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";

    SZM::AI::Legendary::LegendaryAIAssistant::GetInstance().Initialize();
    m_IsInitialized = true;

    std::cout << "[SYSTEM] AI Operating System initialized successfully\n";
    std::cout << "[SYSTEM] Awaiting creator authentication...\n\n";
}

void AIOperatingSystem::Shutdown() {
    std::cout << "[SYSTEM] Shutting down AI Operating System...\n";
    m_IsInitialized = false;
}

bool AIOperatingSystem::EnterCreatorMode(const std::string& password, const std::string& deviceId) {
    return CreatorModeManager::GetInstance().AuthenticateCreator(password, deviceId);
}

bool AIOperatingSystem::IsInCreatorMode() const {
    return CreatorModeManager::GetInstance().IsAuthenticated();
}

CreatorModeManager& AIOperatingSystem::GetCreatorMode() {
    return CreatorModeManager::GetInstance();
}

SZM::AI::Legendary::LegendaryAIAssistant& AIOperatingSystem::GetAI() {
    return SZM::AI::Legendary::LegendaryAIAssistant::GetInstance();
}

std::string AIOperatingSystem::ExecuteCommand(const std::string& command) {
    if (!IsInCreatorMode()) {
        return "[ERROR] Creator mode required to execute commands";
    }

    std::cout << "[COMMAND] " << command << "\n";

    // Parse and execute command
    if (command.find("analyze") == 0) {
        return "[AI] Analyzing project architecture...";
    } else if (command.find("optimize") == 0) {
        return "[AI] Optimizing system...";
    } else if (command.find("train") == 0) {
        return "[AI] Starting training...";
    } else if (command.find("modify") == 0) {
        return "[AI] Proposing modification...";
    } else if (command.find("status") == 0) {
        return GetStatus();
    }

    return "[ERROR] Unknown command: " + command;
}

std::string AIOperatingSystem::GetStatus() {
    std::string status = "\n";
    status += "╔════════════════════════════════════════════════════════════╗\n";
    status += "║              AI OPERATING SYSTEM STATUS                    ║\n";
    status += "╚════════════════════════════════════════════════════════════╝\n\n";
    
    status += "Creator Mode: " + std::string(IsInCreatorMode() ? "ACTIVE" : "LOCKED") + "\n";
    status += "AI Status: OPERATIONAL\n";
    status += "Memory System: ACTIVE\n";
    status += "Agent System: READY\n";
    status += "Project Awareness: INDEXED\n";
    status += "Modification System: ARMED\n";
    status += "Training Studio: READY\n";
    status += "Permission Level: " + std::to_string(static_cast<int>(CreatorModeManager::GetInstance().GetPermissionLevel())) + "\n";
    status += "\n";

    return status;
}

void AIOperatingSystem::EmergencyStop() {
    std::cout << "\n🛑 EMERGENCY STOP ACTIVATED 🛑\n";
    CreatorModeManager::GetInstance().EmergencyStop();
    Shutdown();
}

} // namespace SZM::AI
