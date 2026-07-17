#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace SZM {

    struct SimulationComponent;

    struct DigitalTwinInstance {
        std::string assetUUID;          // Physical asset identifier
        uint32_t    virtualComponentId; // Link to SimulationComponent ID
        std::string operationalState;   // e.g., "Active", "Maintenance", "Offline"
        double      lastSyncTime;       // Timestamp of last telemetry update
    };

    /**
     * @class DigitalTwinManager
     * @brief Manages the lifecycle of digital twin instances, linking physical assets to virtual models.
     */
    class DigitalTwinManager {
    public:
        static DigitalTwinManager& GetInstance() {
            static DigitalTwinManager instance;
            return instance;
        }

        DigitalTwinManager(const DigitalTwinManager&)            = delete;
        DigitalTwinManager& operator=(const DigitalTwinManager&) = delete;

        void Initialize();
        void Shutdown();

        /// Register a physical asset and link it to a virtual component
        bool RegisterAsset(const std::string& assetUUID, uint32_t componentId);

        /// Remove an asset from the digital twin ecosystem
        void UnregisterAsset(const std::string& assetUUID);

        /// Update the operational state of a physical asset
        void UpdateOperationalState(const std::string& assetUUID, const std::string& state);

        /// Get twin info by asset UUID
        const DigitalTwinInstance* GetTwinByAssetUUID(const std::string& assetUUID) const;

        /// Get twin info by virtual component ID
        const DigitalTwinInstance* GetTwinByComponentID(uint32_t componentId) const;

        /// Get all registered twins
        const std::unordered_map<std::string, DigitalTwinInstance>& GetAllTwins() const {
            return m_Twins;
        }

    private:
        DigitalTwinManager() = default;
        ~DigitalTwinManager() = default;

        std::unordered_map<std::string, DigitalTwinInstance> m_Twins;
        bool m_IsInitialized = false;
    };

} // namespace SZM
