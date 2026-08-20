#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <functional>

namespace SZM {

    struct SensorDataPoint {
        double timestamp;
        std::string sensorType; // e.g., "Temperature", "Strain", "Vibration"
        double value;
    };

    struct TelemetryStatistics {
        double min = 0.0;
        double max = 0.0;
        double average = 0.0;
        double stdDev = 0.0;
        size_t count = 0;
    };

    struct AlertThreshold {
        std::string sensorType;
        double warningLow = 0.0;
        double warningHigh = 0.0;
        double criticalLow = 0.0;
        double criticalHigh = 0.0;
    };

    struct TelemetryStream {
        std::string streamId;
        std::string assetUUID;
        std::vector<SensorDataPoint> history;
        double currentAverage;
        std::unordered_map<std::string, AlertThreshold> thresholds;
        double maxHistorySize = 10000; // Maximum data points to keep
    };

    /**
     * @class SensorTelemetry
     * @brief Ingests and processes time-series data from physical sensors
     * Supports persistent storage, statistical analysis, and WebSocket streaming
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

        /// Delete a telemetry stream
        bool DeleteStream(const std::string& streamId);

        /// Push a new data point into a telemetry stream
        void PushData(const std::string& streamId, const std::string& sensorType, double timestamp, double value);

        /// Get the latest aggregated value for a specific stream and sensor type
        double GetLatestValue(const std::string& streamId, const std::string& sensorType) const;

        /// Query data within a time range
        std::vector<SensorDataPoint> QueryByTimeRange(const std::string& streamId, 
                                                       double startTime, double endTime) const;

        /// Get statistics for a sensor type within a stream
        TelemetryStatistics GetStatistics(const std::string& streamId, 
                                          const std::string& sensorType, 
                                          double timeWindowSeconds = 0.0) const;

        /// Set alert thresholds for a sensor
        void SetAlertThreshold(const std::string& streamId, const AlertThreshold& threshold);

        /// Check if any values are in alert state
        bool CheckAlerts(const std::string& streamId) const;

        /// Get all streams
        const std::unordered_map<std::string, TelemetryStream>& GetStreams() const {
            return m_Streams;
        }

        /// Export stream data to JSON
        std::string ExportToJSON(const std::string& streamId) const;

        /// Export stream data to CSV
        std::string ExportToCSV(const std::string& streamId) const;

        /// Import telemetry data from JSON
        bool ImportFromJSON(const std::string& jsonData);

        /// Save all telemetry to persistent storage
        bool SaveToDisk(const std::string& filePath);

        /// Load telemetry from persistent storage
        bool LoadFromDisk(const std::string& filePath);

        /// Register callback for data arrival (for WebSocket/streaming support)
        using DataCallback = std::function<void(const std::string&, const SensorDataPoint&)>;
        void RegisterDataCallback(DataCallback callback);

    private:
        SensorTelemetry() = default;
        ~SensorTelemetry() = default;

        std::unordered_map<std::string, TelemetryStream> m_Streams;
        std::vector<DataCallback> m_DataCallbacks;
        bool m_IsInitialized = false;
        std::string m_StoragePath = "./telemetry_data/";

        // Helper methods
        void PruneHistory(TelemetryStream& stream);
        void NotifyDataArrival(const std::string& streamId, const SensorDataPoint& point);
    };

} // namespace SZM
