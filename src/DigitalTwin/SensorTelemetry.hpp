#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace SZM {

    struct SensorDataPoint {
        double timestamp;
        std::string sensorType; // e.g., "Temperature", "Strain", "Vibration"
        double value;
    };

    struct TelemetryStream {
        std::string streamId;
        std::string assetUUID;
        std::vector<SensorDataPoint> history;
        double currentAverage;
    };

    /**
     * @class SensorTelemetry
     * @brief Ingests and processes time-series data from physical sensors
     */
    class SensorTelemetry {
    public:
        static SensorTelemetry& GetInstance() {
            static SensorTelemetry instance;
            return instance;
        }

        SensorTelemetry(const SensorTelemetry&)            = delete;
        SensorTelemetry& operator=(const SensorTelemetry&) = delete;

        void Initialize();
        void Shutdown();

        /// Create a new telemetry stream linked to a physical asset
        bool CreateStream(const std::string& streamId, const std::string& assetUUID);

        /// Push a new data point into a telemetry stream
        void PushData(const std::string& streamId, const std::string& sensorType, double timestamp, double value);

        /// Get the latest aggregated value for a specific stream and sensor type
        double GetLatestValue(const std::string& streamId, const std::string& sensorType) const;

        /// Get all streams
        const std::unordered_map<std::string, TelemetryStream>& GetStreams() const {
            return m_Streams;
        }

    private:
        SensorTelemetry() = default;
        ~SensorTelemetry() = default;

        std::unordered_map<std::string, TelemetryStream> m_Streams;
        bool m_IsInitialized = false;
    };

} // namespace SZM
