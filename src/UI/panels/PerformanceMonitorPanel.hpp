#pragma once

#include "IPanel.hpp"
#include <vector>
#include <deque>

namespace SZM::UI {

/**
 * @class PerformanceMonitorPanel
 * @brief Monitors and displays performance metrics
 */
class PerformanceMonitorPanel : public IPanel {
public:
    PerformanceMonitorPanel();
    ~PerformanceMonitorPanel() override;

    void Draw() override;
    const char* GetName() const override { return "Performance Monitor"; }

    void RecordFrameTime(float deltaTime);

private:
    std::deque<float> m_FrameTimes;
    static constexpr size_t MAX_HISTORY = 120;

    float GetAverageFrameTime() const;
    float GetMaxFrameTime() const;
    float GetMinFrameTime() const;
    float GetCurrentFPS() const;

    void DrawFrameTimeGraph();
    void DrawMetrics();
};

} // namespace SZM::UI
