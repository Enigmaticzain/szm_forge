#pragma once

#include "CreatorMode.hpp"
#include "LegendaryCore/LegendaryAIAssistant.hpp"
#include <string>
#include <memory>

namespace SZM::AI {

/**
 * @class AIOperatingSystem
 * @brief The central AI operating system for SZM Forge
 * 
 * This is the legendary-tier autonomous engineering intelligence platform.
 * It combines:
 * - Creator mode security
 * - Legendary AI Assistant core
 * - Project awareness
 * - Safe modification system
 * - Training infrastructure
 * - Agent orchestration
 * - Memory systems
 */
class AIOperatingSystem {
public:
    static AIOperatingSystem& GetInstance();

    /**
     * @brief Initialize the AI operating system
     */
    void Initialize();

    /**
     * @brief Shutdown the AI operating system
     */
    void Shutdown();

    /**
     * @brief Enter creator mode
     */
    bool EnterCreatorMode(const std::string& password, const std::string& deviceId);

    /**
     * @brief Check if in creator mode
     */
    bool IsInCreatorMode() const;

    /**
     * @brief Get creator mode manager
     */
    CreatorModeManager& GetCreatorMode();

    /**
     * @brief Get legendary AI assistant
     */
    SZM::AI::Legendary::LegendaryAIAssistant& GetAI();

    /**
     * @brief Execute AI command
     */
    std::string ExecuteCommand(const std::string& command);

    /**
     * @brief Get system status
     */
    std::string GetStatus();

    /**
     * @brief Emergency stop
     */
    void EmergencyStop();

private:
    AIOperatingSystem() = default;
    ~AIOperatingSystem() = default;

    AIOperatingSystem(const AIOperatingSystem&) = delete;
    AIOperatingSystem& operator=(const AIOperatingSystem&) = delete;

    bool m_IsInitialized = false;
};

} // namespace SZM::AI
