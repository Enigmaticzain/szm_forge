#include "DigitalTwinManager.hpp"
#include <iostream>

namespace SZM {

void DigitalTwinManager::Initialize() {
    if (m_IsInitialized) return;
    std::cout << "[DigitalTwinManager] Initializing Digital Twin Core...\n";
    m_IsInitialized = true;
}

void DigitalTwinManager::Shutdown() {
    if (!m_IsInitialized) return;
    std::cout << "[DigitalTwinManager] Shutting down Digital Twin Core...\n";
    std::lock_guard<std::mutex> lock(m_TwinsMutex);
    m_Twins.clear();
    m_IsInitialized = false;
}

bool DigitalTwinManager::RegisterAsset(const std::string& assetUUID, uint32_t componentId) {
    std::lock_guard<std::mutex> lock(m_TwinsMutex);
    if (m_Twins.find(assetUUID) != m_Twins.end()) {
        std::cerr << "[DigitalTwinManager] Asset UUID already registered: " << assetUUID << "\n";
        return false;
    }

    DigitalTwinInstance instance;
    instance.assetUUID = assetUUID;
    instance.virtualComponentId = componentId;
    instance.operationalState = "Active";
    instance.lastSyncTime = 0.0;

    m_Twins[assetUUID] = instance;
    std::cout << "[DigitalTwinManager] Registered asset " << assetUUID << " to component " << componentId << "\n";
    return true;
}

void DigitalTwinManager::UnregisterAsset(const std::string& assetUUID) {
    std::lock_guard<std::mutex> lock(m_TwinsMutex);
    auto it = m_Twins.find(assetUUID);
    if (it != m_Twins.end()) {
        m_Twins.erase(it);
        std::cout << "[DigitalTwinManager] Unregistered asset: " << assetUUID << "\n";
    }
}

void DigitalTwinManager::UpdateOperationalState(const std::string& assetUUID, const std::string& state) {
    std::lock_guard<std::mutex> lock(m_TwinsMutex);
    auto it = m_Twins.find(assetUUID);
    if (it != m_Twins.end()) {
        it->second.operationalState = state;
    }
}

std::optional<DigitalTwinInstance> DigitalTwinManager::GetTwinByAssetUUID(const std::string& assetUUID) const {
    std::lock_guard<std::mutex> lock(m_TwinsMutex);
    auto it = m_Twins.find(assetUUID);
    if (it != m_Twins.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<DigitalTwinInstance> DigitalTwinManager::GetTwinByComponentID(uint32_t componentId) const {
    std::lock_guard<std::mutex> lock(m_TwinsMutex);
    for (const auto& pair : m_Twins) {
        if (pair.second.virtualComponentId == componentId) {
            return pair.second;
        }
    }
    return std::nullopt;
}

} // namespace SZM
