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
    m_Twins.clear();
    m_IsInitialized = false;
}

bool DigitalTwinManager::RegisterAsset(const std::string& assetUUID, uint32_t componentId) {
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
    auto it = m_Twins.find(assetUUID);
    if (it != m_Twins.end()) {
        m_Twins.erase(it);
        std::cout << "[DigitalTwinManager] Unregistered asset: " << assetUUID << "\n";
    }
}

void DigitalTwinManager::UpdateOperationalState(const std::string& assetUUID, const std::string& state) {
    auto it = m_Twins.find(assetUUID);
    if (it != m_Twins.end()) {
        it->second.operationalState = state;
    }
}

const DigitalTwinInstance* DigitalTwinManager::GetTwinByAssetUUID(const std::string& assetUUID) const {
    auto it = m_Twins.find(assetUUID);
    if (it != m_Twins.end()) {
        return &it->second;
    }
    return nullptr;
}

const DigitalTwinInstance* DigitalTwinManager::GetTwinByComponentID(uint32_t componentId) const {
    for (const auto& pair : m_Twins) {
        if (pair.second.virtualComponentId == componentId) {
            return &pair.second;
        }
    }
    return nullptr;
}

} // namespace SZM
