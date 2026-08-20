#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <imgui.h>
#include "../../../src/DigitalTwin/SensorTelemetry.hpp"

namespace SZM::UI {

    /**
     * @class TelemetryPanel
     * @brief ImGui panel for Digital Twin Telemetry visualization and management
     * Displays live sensor data, streams, statistics, and allows data export
     */
    class TelemetryPanel {
    public:
        TelemetryPanel() = default;
        ~TelemetryPanel() = default;

        void Render(bool* isOpen = nullptr);
        
        void SetSelectedStream(const std::string& streamId) { m_SelectedStream = streamId; }
        const std::string& GetSelectedStream() const { return m_SelectedStream; }

    private:
        std::string m_SelectedStream;
        std::string m_NewStreamId;
        std::string m_NewAssetUUID;
        std::map<std::string, bool> m_ExpandedStreams;
        
        // Data visualization
        std::vector<float> m_PlotData;
        std::string m_SelectedSensorType;
        float m_TimeWindowSeconds = 300.0f; // 5 minutes default
        
        // Pagination
        int m_CurrentPage = 0;
        static constexpr int ITEMS_PER_PAGE = 10;
        
        // Telemetry reference
        SensorTelemetry& m_Telemetry = SensorTelemetry::GetInstance();

        // Rendering helpers
        void RenderStreamCreationSection();
        void RenderStreamsListSection();
        void RenderStreamDetailsSection();
        void RenderStatisticsSection();
        void RenderExportSection();
        void RenderDataVisualization();
        
        // Data helpers
        void UpdatePlotData();
        void ExportStreamToJSON(const std::string& streamId);
        void ExportStreamToCSV(const std::string& streamId);
    };

} // namespace SZM::UI
