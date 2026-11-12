#include "SensorTelemetry.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <sstream>
#include <nlohmann/json.hpp>

namespace SZM {

void SensorTelemetry::Initialize() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_IsInitialized) return;
    std::cout << "[SensorTelemetry] Initializing telemetry ingestion engine...\n";
    m_IsInitialized = true;
}

void SensorTelemetry::Shutdown() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_IsInitialized) return;
    std::cout << "[SensorTelemetry] Shutting down telemetry engine...\n";
    m_Streams.clear();
    m_DataCallbacks.clear();
    m_IsInitialized = false;
}

bool SensorTelemetry::CreateStreamUnlocked(const std::string& streamId, const std::string& assetUUID) {
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

bool SensorTelemetry::CreateStream(const std::string& streamId, const std::string& assetUUID) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return CreateStreamUnlocked(streamId, assetUUID);
}

bool SensorTelemetry::DeleteStream(const std::string& streamId) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Streams.find(streamId);
    if (it != m_Streams.end()) {
        m_Streams.erase(it);
        std::cout << "[SensorTelemetry] Deleted stream " << streamId << "\n";
        return true;
    }
    return false;
}

std::optional<SensorDataPoint> SensorTelemetry::PushDataUnlocked(const std::string& streamId,
                                                                 const std::string& sensorType,
                                                                 double timestamp,
                                                                 double value) {
    auto it = m_Streams.find(streamId);
    if (it == m_Streams.end()) {
        return std::nullopt;
    }

    SensorDataPoint point{timestamp, sensorType, value};
    it->second.history.push_back(point);

    // Simple rolling average over the last 10 points of the same type
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

    PruneHistory(it->second);
    return point;
}

void SensorTelemetry::PushData(const std::string& streamId, const std::string& sensorType, double timestamp, double value) {
    std::optional<SensorDataPoint> pushed;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        pushed = PushDataUnlocked(streamId, sensorType, timestamp, value);
    }
    if (pushed.has_value()) {
        NotifyDataArrival(streamId, *pushed);
    }
}

double SensorTelemetry::GetLatestValueUnlocked(const TelemetryStream& stream, const std::string& sensorType) const {
    for (auto rit = stream.history.rbegin(); rit != stream.history.rend(); ++rit) {
        if (rit->sensorType == sensorType) {
            return rit->value;
        }
    }
    return 0.0;
}

double SensorTelemetry::GetLatestValue(const std::string& streamId, const std::string& sensorType) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Streams.find(streamId);
    if (it != m_Streams.end()) {
        return GetLatestValueUnlocked(it->second, sensorType);
    }
    return 0.0;
}

std::vector<SensorDataPoint> SensorTelemetry::QueryByTimeRange(const std::string& streamId, 
                                                                double startTime, double endTime) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<SensorDataPoint> result;
    auto it = m_Streams.find(streamId);
    if (it != m_Streams.end()) {
        for (const auto& point : it->second.history) {
            if (point.timestamp >= startTime && point.timestamp <= endTime) {
                result.push_back(point);
            }
        }
    }
    return result;
}

TelemetryStatistics SensorTelemetry::GetStatistics(const std::string& streamId, 
                                                    const std::string& sensorType,
                                                    double timeWindowSeconds) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    TelemetryStatistics stats;
    auto it = m_Streams.find(streamId);
    if (it == m_Streams.end()) return stats;

    std::vector<double> values;
    double currentTime = 0.0;
    
    // Get the current time (last timestamp in history)
    if (!it->second.history.empty()) {
        currentTime = it->second.history.back().timestamp;
    }

    double startTime = (timeWindowSeconds > 0) ? (currentTime - timeWindowSeconds) : 0.0;

    for (const auto& point : it->second.history) {
        if (point.sensorType == sensorType && point.timestamp >= startTime) {
            values.push_back(point.value);
        }
    }

    if (!values.empty()) {
        stats.count = values.size();
        stats.min = *std::min_element(values.begin(), values.end());
        stats.max = *std::max_element(values.begin(), values.end());
        stats.average = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
        
        // Calculate standard deviation
        double sumSquaredDiff = 0.0;
        for (double val : values) {
            double diff = val - stats.average;
            sumSquaredDiff += diff * diff;
        }
        stats.stdDev = std::sqrt(sumSquaredDiff / values.size());
    }

    return stats;
}

void SensorTelemetry::SetAlertThreshold(const std::string& streamId, const AlertThreshold& threshold) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Streams.find(streamId);
    if (it != m_Streams.end()) {
        it->second.thresholds[threshold.sensorType] = threshold;
        std::cout << "[SensorTelemetry] Alert threshold set for " << threshold.sensorType 
                  << " in stream " << streamId << "\n";
    }
}

bool SensorTelemetry::CheckAlerts(const std::string& streamId) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Streams.find(streamId);
    if (it == m_Streams.end()) return false;

    for (const auto& [sensorType, threshold] : it->second.thresholds) {
        double latestValue = GetLatestValueUnlocked(it->second, sensorType);
        
        if (latestValue < threshold.criticalLow || latestValue > threshold.criticalHigh) {
            return true;
        }
    }
    return false;
}

std::string SensorTelemetry::ExportToJSON(const std::string& streamId) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Streams.find(streamId);
    if (it == m_Streams.end()) return "{}";

    std::ostringstream json;
    json << "{\n";
    json << "  \"streamId\": \"" << it->second.streamId << "\",\n";
    json << "  \"assetUUID\": \"" << it->second.assetUUID << "\",\n";
    json << "  \"dataPoints\": [\n";
    
    for (size_t i = 0; i < it->second.history.size(); ++i) {
        const auto& point = it->second.history[i];
        json << "    {\n";
        json << "      \"timestamp\": " << point.timestamp << ",\n";
        json << "      \"sensorType\": \"" << point.sensorType << "\",\n";
        json << "      \"value\": " << point.value << "\n";
        json << "    }";
        if (i < it->second.history.size() - 1) json << ",";
        json << "\n";
    }
    
    json << "  ]\n";
    json << "}\n";
    return json.str();
}

std::string SensorTelemetry::ExportToCSV(const std::string& streamId) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Streams.find(streamId);
    if (it == m_Streams.end()) return "";

    std::ostringstream csv;
    csv << "timestamp,sensorType,value\n";
    
    for (const auto& point : it->second.history) {
        csv << point.timestamp << "," << point.sensorType << "," << point.value << "\n";
    }
    
    return csv.str();
}

bool SensorTelemetry::ImportFromJSON(const std::string& jsonData) {
    try {
        auto parsed = nlohmann::json::parse(jsonData);
        std::vector<std::pair<std::string, SensorDataPoint>> pushedEvents;

        auto ingestObject = [&](const nlohmann::json& obj) {
            std::string streamId = obj.value("streamId", "");
            if (streamId.empty()) return;
            std::string assetUUID = obj.value("assetUUID", "");
            if (m_Streams.find(streamId) == m_Streams.end()) {
                CreateStreamUnlocked(streamId, assetUUID);
            }
            if (obj.contains("data") && obj["data"].is_array()) {
                for (const auto& dp : obj["data"]) {
                    double ts = dp.value("timestamp", 0.0);
                    std::string st = dp.value("sensorType", "Generic");
                    double val = dp.value("value", 0.0);
                    if (auto pt = PushDataUnlocked(streamId, st, ts, val)) {
                        pushedEvents.emplace_back(streamId, *pt);
                    }
                }
            }
        };

        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (parsed.is_array()) {
                for (const auto& item : parsed) {
                    ingestObject(item);
                }
            } else if (parsed.is_object()) {
                ingestObject(parsed);
            }
        }

        for (const auto& [streamId, pt] : pushedEvents) {
            NotifyDataArrival(streamId, pt);
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SensorTelemetry] JSON parse error: " << e.what() << "\n";
        return false;
    }
}

bool SensorTelemetry::SaveToDisk(const std::string& filePath) {
    try {
        nlohmann::json outArr = nlohmann::json::array();
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            for (const auto& [streamId, stream] : m_Streams) {
                nlohmann::json sObj;
                sObj["streamId"] = streamId;
                sObj["assetUUID"] = stream.assetUUID;
                nlohmann::json pts = nlohmann::json::array();
                for (const auto& p : stream.history) {
                    pts.push_back({
                        {"timestamp", p.timestamp},
                        {"sensorType", p.sensorType},
                        {"value", p.value}
                    });
                }
                sObj["data"] = pts;
                outArr.push_back(sObj);
            }
        }

        std::ofstream file(filePath);
        if (!file.is_open()) return false;
        file << outArr.dump(2);
        file.close();
        std::cout << "[SensorTelemetry] Saved telemetry to " << filePath << "\n";
        return true;
    } catch (...) {
        std::cerr << "[SensorTelemetry] Failed to save telemetry to " << filePath << "\n";
        return false;
    }
}

bool SensorTelemetry::LoadFromDisk(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return ImportFromJSON(buffer.str());
    } catch (...) {
        std::cerr << "[SensorTelemetry] Failed to load telemetry from " << filePath << "\n";
        return false;
    }
}

void SensorTelemetry::RegisterDataCallback(DataCallback callback) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_DataCallbacks.push_back(std::move(callback));
}

void SensorTelemetry::PruneHistory(TelemetryStream& stream) {
    if (stream.history.size() > stream.maxHistorySize) {
        // Keep only the most recent data
        size_t removeCount = stream.history.size() - stream.maxHistorySize;
        stream.history.erase(stream.history.begin(), stream.history.begin() + removeCount);
    }
}

void SensorTelemetry::NotifyDataArrival(const std::string& streamId, const SensorDataPoint& point) {
    std::vector<DataCallback> callbacksCopy;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        callbacksCopy = m_DataCallbacks;
    }
    for (auto& callback : callbacksCopy) {
        if (callback) {
            callback(streamId, point);
        }
    }
}

} // namespace SZM
