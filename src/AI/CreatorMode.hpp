#pragma once

#include <string>
#include <cstdint>
#include <chrono>
#include <vector>
#include <unordered_set>

namespace SZM::AI {

/**
 * @enum PermissionLevel
 * @brief Role hierarchy for AI system access
 */
enum class PermissionLevel : uint8_t {
    VIEWER = 1,           // Read-only access
    DEVELOPER = 2,        // Code viewing + basic modifications
    ENGINEER = 3,         // Full development access
    AI_ARCHITECT = 4,     // AI system configuration
    ROOT_CREATOR = 5      // Unrestricted access + AI modification
};

/**
 * @enum Permission
 * @brief Granular permission flags
 */
enum class Permission : uint32_t {
    READ_FILES = 1 << 0,
    WRITE_FILES = 1 << 1,
    EXECUTE_COMMANDS = 1 << 2,
    MODIFY_AI = 1 << 3,
    TRAIN_AI = 1 << 4,
    SYSTEM_ACCESS = 1 << 5,
    NETWORK_ACCESS = 1 << 6,
    ROOT_ACCESS = 1 << 7,
    EMERGENCY_STOP = 1 << 8
};

/**
 * @struct CreatorSession
 * @brief Authenticated creator session
 */
struct CreatorSession {
    uint64_t sessionId;
    PermissionLevel level;
    uint32_t permissions;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point expiresAt;
    std::string deviceId;
    bool isActive;
    bool requiresApproval;
};

/**
 * @class CreatorModeManager
 * @brief Manages creator authentication and permissions
 */
class CreatorModeManager {
public:
    static CreatorModeManager& GetInstance();

    /**
     * @brief Authenticate creator with password
     */
    bool AuthenticateCreator(const std::string& password, const std::string& deviceId);

    /**
     * @brief Check if creator is authenticated
     */
    bool IsAuthenticated() const;

    /**
     * @brief Get current permission level
     */
    PermissionLevel GetPermissionLevel() const;

    /**
     * @brief Check if specific permission is granted
     */
    bool HasPermission(Permission perm) const;

    /**
     * @brief Request permission escalation
     */
    bool RequestPermissionEscalation(Permission perm);

    /**
     * @brief Enable AI modification mode
     */
    bool EnableAIModificationMode(const std::string& confirmationCode);

    /**
     * @brief Disable AI modification mode
     */
    void DisableAIModificationMode();

    /**
     * @brief Emergency stop all AI operations
     */
    void EmergencyStop();

    /**
     * @brief Get current session
     */
    const CreatorSession& GetCurrentSession() const;

    /**
     * @brief Logout creator
     */
    void Logout();

private:
    CreatorModeManager() = default;
    ~CreatorModeManager() = default;

    CreatorModeManager(const CreatorModeManager&) = delete;
    CreatorModeManager& operator=(const CreatorModeManager&) = delete;

    CreatorSession m_CurrentSession;
    bool m_IsAuthenticated = false;
    bool m_AIModificationEnabled = false;
    std::vector<std::string> m_ActionLog;

    bool ValidatePassword(const std::string& password) const;
    bool ValidateDeviceBinding(const std::string& deviceId) const;
    void LogAction(const std::string& action);
};

} // namespace SZM::AI
