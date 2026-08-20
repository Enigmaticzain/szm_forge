#include "TelemetryPanel.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <set>
#include <imgui.h>
#include <imgui_internal.h>

namespace SZM::UI {

void TelemetryPanel::Render(bool* isOpen) {
    if (!ImGui::Begin("Telemetry Manager##TelemetryPanel", isOpen, ImGuiWindowFlags_NoMove)) {
        ImGui::End();
        return;
    }

    // Set minimum window size
    ImGui::SetWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);

    // Render tabs
    static int selectedTab = 0;
    if (ImGui::BeginTabBar("TelemetryTabs")) {
        if (ImGui::BeginTabItem("Streams")) {
            RenderStreamCreationSection();
            ImGui::Spacing();
            RenderStreamsListSection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Details")) {
            if (!m_SelectedStream.empty()) {
                RenderStreamDetailsSection();
                ImGui::Spacing();
                RenderStatisticsSection();
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Select a stream to view details");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visualization")) {
            if (!m_SelectedStream.empty()) {
                RenderDataVisualization();
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Select a stream to visualize data");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Export")) {
            if (!m_SelectedStream.empty()) {
                RenderExportSection();
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Select a stream to export data");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void TelemetryPanel::RenderStreamCreationSection() {
    ImGui::SeparatorText("Create New Stream");
    
    static char streamIdBuf[128] = "stream-01";
    static char assetUUIDBuf[256] = "asset-uuid-12345";

    ImGui::Text("Stream Configuration");
    ImGui::InputTextWithHint("##StreamID", "Stream ID (e.g., machine-01)", streamIdBuf, IM_ARRAYSIZE(streamIdBuf));
    ImGui::InputTextWithHint("##AssetUUID", "Asset UUID", assetUUIDBuf, IM_ARRAYSIZE(assetUUIDBuf));

    if (ImGui::Button("Create Stream##CreateBtn", ImVec2(-1, 0))) {
        std::string streamId(streamIdBuf);
        std::string assetUUID(assetUUIDBuf);
        
        if (!streamId.empty() && !assetUUID.empty()) {
            if (m_Telemetry.CreateStream(streamId, assetUUID)) {
                strcpy(streamIdBuf, "stream-01");
                strcpy(assetUUIDBuf, "asset-uuid-12345");
            }
        }
    }
}

void TelemetryPanel::RenderStreamsListSection() {
    ImGui::SeparatorText("Active Streams");

    const auto& streams = m_Telemetry.GetStreams();
    
    if (streams.empty()) {
        ImGui::TextDisabled("No active streams. Create one to get started.");
        return;
    }

    if (ImGui::BeginTable("StreamsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Stream ID", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Data Points", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableHeadersRow();

        for (const auto& [streamId, stream] : streams) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            
            bool isSelected = (m_SelectedStream == streamId);
            if (ImGui::Selectable(streamId.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                m_SelectedStream = streamId;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%zu", stream.history.size());

            ImGui::TableSetColumnIndex(2);
            ImGui::PushID(streamId.c_str());
            if (ImGui::SmallButton("Delete")) {
                m_Telemetry.DeleteStream(streamId);
                if (m_SelectedStream == streamId) {
                    m_SelectedStream.clear();
                }
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void TelemetryPanel::RenderStreamDetailsSection() {
    const auto& streams = m_Telemetry.GetStreams();
    auto it = streams.find(m_SelectedStream);
    if (it == streams.end()) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Stream not found");
        return;
    }

    const auto& stream = it->second;

    ImGui::SeparatorText(("Stream: " + stream.streamId).c_str());

    // Basic Info
    ImGui::LabelText("Stream ID", "%s", stream.streamId.c_str());
    ImGui::LabelText("Asset UUID", "%s", stream.assetUUID.c_str());
    ImGui::LabelText("Total Data Points", "%zu", stream.history.size());
    ImGui::LabelText("Current Average", "%.4f", stream.currentAverage);

    // Recent Data
    ImGui::SeparatorText("Recent Data Points");
    
    int startIdx = std::max(0, static_cast<int>(stream.history.size()) - 10);
    
    if (ImGui::BeginTable("RecentDataTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Sensor Type", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableHeadersRow();

        for (size_t i = startIdx; i < stream.history.size(); ++i) {
            const auto& point = stream.history[i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%.2f", point.timestamp);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", point.sensorType.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.4f", point.value);
        }

        ImGui::EndTable();
    }
}

void TelemetryPanel::RenderStatisticsSection() {
    const auto& streams = m_Telemetry.GetStreams();
    auto it = streams.find(m_SelectedStream);
    if (it == streams.end()) return;

    const auto& stream = it->second;

    ImGui::SeparatorText("Statistics");

    // Get unique sensor types
    std::set<std::string> sensorTypes;
    for (const auto& point : stream.history) {
        sensorTypes.insert(point.sensorType);
    }

    if (ImGui::BeginCombo("##SensorType", m_SelectedSensorType.empty() ? "Select Sensor Type" : m_SelectedSensorType.c_str())) {
        for (const auto& sensorType : sensorTypes) {
            if (ImGui::Selectable(sensorType.c_str(), m_SelectedSensorType == sensorType)) {
                m_SelectedSensorType = sensorType;
            }
        }
        ImGui::EndCombo();
    }

    if (!m_SelectedSensorType.empty()) {
        auto stats = m_Telemetry.GetStatistics(m_SelectedStream, m_SelectedSensorType);
        
        if (stats.count > 0) {
            ImGui::Separator();
            ImGui::LabelText("Count", "%zu", stats.count);
            ImGui::LabelText("Min", "%.6f", stats.min);
            ImGui::LabelText("Max", "%.6f", stats.max);
            ImGui::LabelText("Average", "%.6f", stats.average);
            ImGui::LabelText("Std Dev", "%.6f", stats.stdDev);
        }
    }
}

void TelemetryPanel::RenderDataVisualization() {
    const auto& streams = m_Telemetry.GetStreams();
    auto it = streams.find(m_SelectedStream);
    if (it == streams.end()) return;

    const auto& stream = it->second;

    ImGui::Text("Time Window (seconds):");
    ImGui::SliderFloat("##TimeWindow", &m_TimeWindowSeconds, 60.0f, 3600.0f);

    // Get unique sensor types
    std::set<std::string> sensorTypes;
    for (const auto& point : stream.history) {
        sensorTypes.insert(point.sensorType);
    }

    if (ImGui::BeginCombo("##SensorTypeViz", m_SelectedSensorType.empty() ? "Select Sensor Type" : m_SelectedSensorType.c_str())) {
        for (const auto& sensorType : sensorTypes) {
            if (ImGui::Selectable(sensorType.c_str(), m_SelectedSensorType == sensorType)) {
                m_SelectedSensorType = sensorType;
            }
        }
        ImGui::EndCombo();
    }

    if (!m_SelectedSensorType.empty()) {
        UpdatePlotData();
        
        ImGui::SeparatorText(("Plot: " + m_SelectedSensorType).c_str());
        
        if (!m_PlotData.empty()) {
            ImGui::PlotLines(
                "##DataPlot",
                m_PlotData.data(),
                static_cast<int>(m_PlotData.size()),
                0,
                nullptr,
                FLT_MAX,
                FLT_MAX,
                ImVec2(-1, 200)
            );
        } else {
            ImGui::TextDisabled("No data available for this time window");
        }
    }
}

void TelemetryPanel::RenderExportSection() {
    ImGui::SeparatorText("Export Data");

    if (ImGui::Button("Export as JSON##ExportJSON", ImVec2(ImGui::GetContentRegionAvail().x * 0.48f, 0))) {
        ExportStreamToJSON(m_SelectedStream);
    }

    ImGui::SameLine();

    if (ImGui::Button("Export as CSV##ExportCSV", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        ExportStreamToCSV(m_SelectedStream);
    }

    ImGui::Spacing();
    ImGui::TextWrapped("Exported files will be saved to the telemetry_exports directory with timestamps.");
}

void TelemetryPanel::UpdatePlotData() {
    m_PlotData.clear();

    const auto& streams = m_Telemetry.GetStreams();
    auto it = streams.find(m_SelectedStream);
    if (it == streams.end()) return;

    auto dataPoints = m_Telemetry.QueryByTimeRange(
        m_SelectedStream,
        0,
        1e10 // Get all within the memory
    );

    // Filter by sensor type and time window
    double currentTime = 0;
    if (!dataPoints.empty()) {
        currentTime = dataPoints.back().timestamp;
    }
    double startTime = currentTime - m_TimeWindowSeconds;

    std::vector<double> values;
    for (const auto& point : dataPoints) {
        if (point.sensorType == m_SelectedSensorType && point.timestamp >= startTime) {
            values.push_back(point.value);
        }
    }

    // Convert to float for ImGui plotting
    m_PlotData.reserve(values.size());
    for (double val : values) {
        m_PlotData.push_back(static_cast<float>(val));
    }
}

void TelemetryPanel::ExportStreamToJSON(const std::string& streamId) {
    try {
        std::string jsonData = m_Telemetry.ExportToJSON(streamId);
        
        // Create exports directory
        system("mkdir -p telemetry_exports");
        
        // Generate filename with timestamp
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "telemetry_exports/%Y%m%d_%H%M%S_") << streamId << ".json";
        std::string filename = oss.str();

        std::ofstream file(filename);
        if (file.is_open()) {
            file << jsonData;
            file.close();
            std::cout << "[TelemetryPanel] Exported JSON to " << filename << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[TelemetryPanel] Failed to export JSON: " << e.what() << "\n";
    }
}

void TelemetryPanel::ExportStreamToCSV(const std::string& streamId) {
    try {
        std::string csvData = m_Telemetry.ExportToCSV(streamId);
        
        // Create exports directory
        system("mkdir -p telemetry_exports");
        
        // Generate filename with timestamp
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "telemetry_exports/%Y%m%d_%H%M%S_") << streamId << ".csv";
        std::string filename = oss.str();

        std::ofstream file(filename);
        if (file.is_open()) {
            file << csvData;
            file.close();
            std::cout << "[TelemetryPanel] Exported CSV to " << filename << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[TelemetryPanel] Failed to export CSV: " << e.what() << "\n";
    }
}

} // namespace SZM::UI
