#include "PerformanceMonitorPanel.hpp"
#include <imgui.h>
#include <algorithm>
#include <numeric>

namespace SZM::UI {

PerformanceMonitorPanel::PerformanceMonitorPanel() = default;

PerformanceMonitorPanel::~PerformanceMonitorPanel() = default;

void PerformanceMonitorPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen)) {
        ImGui::End();
        return;
    }

    DrawMetrics();
    ImGui::Separator();
    DrawFrameTimeGraph();

    ImGui::End();
}

void PerformanceMonitorPanel::RecordFrameTime(float deltaTime) {
    m_FrameTimes.push_back(deltaTime);
    if (m_FrameTimes.size() > MAX_HISTORY) {
        m_FrameTimes.pop_front();
    }
}

float PerformanceMonitorPanel::GetAverageFrameTime() const {
    if (m_FrameTimes.empty()) return 0.0f;
    return std::accumulate(m_FrameTimes.begin(), m_FrameTimes.end(), 0.0f) / m_FrameTimes.size();
}

float PerformanceMonitorPanel::GetMaxFrameTime() const {
    if (m_FrameTimes.empty()) return 0.0f;
    return *std::max_element(m_FrameTimes.begin(), m_FrameTimes.end());
}

float PerformanceMonitorPanel::GetMinFrameTime() const {
    if (m_FrameTimes.empty()) return 0.0f;
    return *std::min_element(m_FrameTimes.begin(), m_FrameTimes.end());
}

float PerformanceMonitorPanel::GetCurrentFPS() const {
    float avgTime = GetAverageFrameTime();
    return avgTime > 0.0f ? 1.0f / avgTime : 0.0f;
}

void PerformanceMonitorPanel::DrawMetrics() {
    ImGui::Text("Performance Metrics:");
    ImGui::Indent();

    ImGui::BulletText("FPS: %.1f", GetCurrentFPS());
    ImGui::BulletText("Avg Frame Time: %.2f ms", GetAverageFrameTime() * 1000.0f);
    ImGui::BulletText("Max Frame Time: %.2f ms", GetMaxFrameTime() * 1000.0f);
    ImGui::BulletText("Min Frame Time: %.2f ms", GetMinFrameTime() * 1000.0f);

    ImGui::Unindent();
}

void PerformanceMonitorPanel::DrawFrameTimeGraph() {
    ImGui::Text("Frame Time History:");

    std::vector<float> values(m_FrameTimes.begin(), m_FrameTimes.end());
    ImGui::PlotLines(
        "##FrameTimeGraph",
        values.data(),
        values.size(),
        0,
        nullptr,
        0.0f,
        GetMaxFrameTime() * 1.5f,
        ImVec2(0, 100)
    );
}

} // namespace SZM::UI
