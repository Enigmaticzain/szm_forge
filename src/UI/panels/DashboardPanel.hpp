#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/SimulationEngine.hpp"
#include <imgui.h>
#include <deque>
#include <chrono>

/**
 * @class DashboardPanel
 * @brief Real-time system metrics and performance monitoring dashboard.
 *
 * Features:
 *  - FPS and frame time display
 *  - Component statistics (count, stress/temp averages)
 *  - System performance metrics
 *  - Real-time trend graphs
 *  - Simulation status and warnings
 *  - System health indicators
 */
class DashboardPanel : public IPanel {
public:
    DashboardPanel();
    ~DashboardPanel() override = default;

    void        Draw()     override;
    std::string GetTitle() const override { return "Dashboard"; }

private:
    void RenderMetricCards();
    // Rendering sections
    void RenderPerformanceMetrics();
    void RenderComponentStatistics();
    void RenderSimulationStatus();
    void RenderSystemHealth();
    void RenderTrendGraphs();
    void RenderWarnings();
    
    // Data collection
    void UpdateMetrics();
    void RecordMetricSample();
    
    // Utility
    const char* GetHealthStatus(float ratio) const;
    ImU32 GetHealthColor(float ratio) const;

private:
    // Performance tracking
    std::deque<float> m_FpsHistory;
    std::deque<float> m_FrameTimeHistory;
    std::deque<float> m_StressAverageHistory;
    std::deque<float> m_TempAverageHistory;
    
    static constexpr size_t MAX_HISTORY = 120; // ~2 seconds at 60 FPS
    
    // Current metrics
    float m_CurrentFps = 0.0f;
    float m_CurrentFrameTime = 0.0f;
    float m_AverageStress = 0.0f;
    float m_AverageTempRatio = 0.0f;
    float m_MaxStress = 0.0f;
    float m_MaxTempRatio = 0.0f;
    
    // Component statistics
    size_t m_ComponentCount = 0;
    size_t m_DangerousComponentCount = 0;
    
    // Timing
    std::chrono::time_point<std::chrono::high_resolution_clock> m_LastUpdateTime;
    double m_TimeSinceLastSample = 0.0;
    static constexpr double SAMPLE_INTERVAL = 0.016; // ~60 FPS
    
    // Display options
    bool m_ShowDetailedMetrics = false;
    bool m_ShowGraphs = true;
    bool m_ShowWarnings = true;
};
