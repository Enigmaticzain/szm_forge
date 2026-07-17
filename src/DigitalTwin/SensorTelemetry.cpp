#include "SensorTelemetry.hpp"
#include <iostream>

namespace SZM {

void SensorTelemetry::Initialize() {
    if (m_IsInitialized) return;
    std::cout << "[SensorTelemetry] Initializing telemetry ingestion engine...\n";
    m_IsInitialized = true;
}

void SensorTelemetry::Shutdown() {
    if (!m_IsInitialized) return;
    std::cout << "[SensorTelemetry] Shutting down telemetry engine...\n";
    m_Streams.clear();
    m_IsInitialized = false;
}

bool SensorTelemetry::CreateStream(const std::string& streamId, const std::string& assetUUID) {
    if (m_Streams.find(streamId) != m_Streams.end()) {
        std::cerr << "[SensorTelemetry] Stream ID already exists: " << streamId << "\n";
        return false;
    }

    TelemetryStream stream;
    stream.streamId = streamId;
    stream.assetUUID = assetUUID;
    stream.currentAverage = 0.0;

    m_Streams[streamId] = stream;
    std::cout << "[SensorTelemetry] Created stream " << streamId << " for asset " << assetUUID << "\n";
    return true;
}

void SensorTelemetry::PushData(const std::string& streamId, const std::string& sensorType, double timestamp, double value) {
    auto it = m_Streams.find(streamId);
    if (it != m_Streams.end()) {
        it->second.history.push_back({timestamp, sensorType, value});
        
        // Simple rolling average over the last 10 points
        double sum = 0.0;
        int count = 0;
        int maxPoints = 10;
        int startIdx = std::max(0, static_cast<int>(it->second.history.size()) - maxPoints);
        
        for (size_t i = startIdx; i < it->second.history.size(); ++i) {
            if (it->second.history[i].sensorType == sensorType) {
                sum += it->second.history[i].value;
                count++;
            }
        }
        
        if (count > 0) {
            it->second.currentAverage = sum / count;
        }
    }
}

double SensorTelemetry::GetLatestValue(const std::string& streamId, const std::string& sensorType) const {
    auto it = m_Streams.find(streamId);
    if (it != m_Streams.end()) {
        for (auto rit = it->second.history.rbegin(); rit != it->second.history.rend(); ++rit) {
            if (rit->sensorType == sensorType) {
                return rit->value;
            }
        }
    }
    return 0.0;
}

} // namespace SZM
