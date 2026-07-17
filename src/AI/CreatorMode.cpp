#include "CreatorMode.hpp"
#include <iostream>
#include <cstring>

namespace SZM::AI {

CreatorModeManager& CreatorModeManager::GetInstance() {
    static CreatorModeManager instance;
    return instance;
}

bool CreatorModeManager::AuthenticateCreator(const std::string& password, const std::string& deviceId) {
    if (!ValidatePassword(password)) {
        LogAction("FAILED_AUTH: Invalid password");
        return false;
    }

    if (!ValidateDeviceBinding(deviceId)) {
        LogAction("FAILED_AUTH: Device not recognized");
        return false;
    }

    m_CurrentSession.sessionId = std::chrono::system_clock::now().time_since_epoch().count();
    m_CurrentSession.level = PermissionLevel::ROOT_CREATOR;
    m_CurrentSession.createdAt = std::chrono::system_clock::now();
    m_CurrentSession.expiresAt = m_CurrentSession.createdAt + std::chrono::hours(8);
    m_CurrentSession.deviceId = deviceId;
    m_CurrentSession.isActive = true;
    m_CurrentSession.requiresApproval = true;

    // Set permissions based on level
    m_CurrentSession.permissions = 
        static_cast<uint32_t>(Permission::READ_FILES) |
        static_cast<uint32_t>(Permission::WRITE_FILES) |
        static_cast<uint32_t>(Permission::EXECUTE_COMMANDS) |
        static_cast<uint32_t>(Permission::TRAIN_AI) |
        static_cast<uint32_t>(Permission::SYSTEM_ACCESS) |
        static_cast<uint32_t>(Permission::EMERGENCY_STOP);

    m_IsAuthenticated = true;
    LogAction("AUTH_SUCCESS: Creator authenticated");
    
    std::cout << "[CreatorMode] Authentication successful\n";
    std::cout << "[CreatorMode] Session ID: " << m_CurrentSession.sessionId << "\n";
    std::cout << "[CreatorMode] Expires in 8 hours\n";

    return true;
}

bool CreatorModeManager::IsAuthenticated() const {
    if (!m_IsAuthenticated) return false;
    
    auto now = std::chrono::system_clock::now();
    return now < m_CurrentSession.expiresAt && m_CurrentSession.isActive;
}

PermissionLevel CreatorModeManager::GetPermissionLevel() const {
    return m_CurrentSession.level;
}

bool CreatorModeManager::HasPermission(Permission perm) const {
    if (!IsAuthenticated()) return false;
    return (m_CurrentSession.permissions & static_cast<uint32_t>(perm)) != 0;
}

bool CreatorModeManager::RequestPermissionEscalation(Permission perm) {
    if (!IsAuthenticated()) {
        LogAction("ESCALATION_DENIED: Not authenticated");
        return false;
    }

    if (perm == Permission::MODIFY_AI || perm == Permission::ROOT_ACCESS) {
        LogAction("ESCALATION_PENDING: Requires confirmation");
        std::cout << "[CreatorMode] Permission escalation required\n";
        std::cout << "[CreatorMode] Awaiting creator confirmation...\n";
        return false;
    }

    m_CurrentSession.permissions |= static_cast<uint32_t>(perm);
    LogAction("ESCALATION_GRANTED: " + std::to_string(static_cast<uint32_t>(perm)));
    return true;
}

bool CreatorModeManager::EnableAIModificationMode(const std::string& confirmationCode) {
    if (!IsAuthenticated()) {
        LogAction("AI_MOD_DENIED: Not authenticated");
        return false;
    }

    if (confirmationCode != "ENABLE_AI_MODIFICATION_MODE") {
        LogAction("AI_MOD_DENIED: Invalid confirmation code");
        return false;
    }

    m_CurrentSession.permissions |= static_cast<uint32_t>(Permission::MODIFY_AI);
    m_CurrentSession.permissions |= static_cast<uint32_t>(Permission::ROOT_ACCESS);
    m_AIModificationEnabled = true;

    LogAction("AI_MODIFICATION_ENABLED");
    std::cout << "[CreatorMode] ⚠️  AI MODIFICATION MODE ENABLED\n";
    std::cout << "[CreatorMode] The AI system can now modify SZM Forge\n";
    std::cout << "[CreatorMode] All changes will be logged and reversible\n";

    return true;
}

void CreatorModeManager::DisableAIModificationMode() {
    m_CurrentSession.permissions &= ~static_cast<uint32_t>(Permission::MODIFY_AI);
    m_CurrentSession.permissions &= ~static_cast<uint32_t>(Permission::ROOT_ACCESS);
    m_AIModificationEnabled = false;

    LogAction("AI_MODIFICATION_DISABLED");
    std::cout << "[CreatorMode] AI modification mode disabled\n";
}

void CreatorModeManager::EmergencyStop() {
    m_IsAuthenticated = false;
    m_AIModificationEnabled = false;
    m_CurrentSession.isActive = false;

    LogAction("EMERGENCY_STOP_ACTIVATED");
    std::cout << "[CreatorMode] 🛑 EMERGENCY STOP ACTIVATED\n";
    std::cout << "[CreatorMode] All AI operations halted\n";
}

const CreatorSession& CreatorModeManager::GetCurrentSession() const {
    return m_CurrentSession;
}

void CreatorModeManager::Logout() {
    m_IsAuthenticated = false;
    m_AIModificationEnabled = false;
    m_CurrentSession.isActive = false;

    LogAction("LOGOUT");
    std::cout << "[CreatorMode] Creator logged out\n";
}

bool CreatorModeManager::ValidatePassword(const std::string& password) const {
    // In production, use proper hashing (bcrypt, argon2)
    // This is a placeholder
    return password.length() >= 12;
}

bool CreatorModeManager::ValidateDeviceBinding(const std::string& deviceId) const {
    // In production, validate against stored device fingerprints
    return !deviceId.empty();
}

void CreatorModeManager::LogAction(const std::string& action) {
    auto timestamp = std::chrono::system_clock::now();
    m_ActionLog.push_back("[" + std::to_string(timestamp.time_since_epoch().count()) + "] " + action);
}

} // namespace SZM::AI
