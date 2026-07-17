#include "DashboardPanel.hpp"
#include "../UIStyleHelper.hpp"
#include "../../Input/EventBus.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <vector>

namespace {

void BeginSurface(const char* id, float height = 0.0f) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::GetSurfaceColor(0)));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::GetBorderColor()));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
    ImGui::BeginChild(id, ImVec2(0.0f, height), true, ImGuiWindowFlags_NoScrollbar);
}

void EndSurface() {
    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void DrawBigMetric(const char* label, const char* value, const char* note, ImU32 accent) {
    ImGui::TextDisabled("%s", label);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(accent));
    ImGui::SetWindowFontScale(1.2f);
    ImGui::Text("%s", value);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::TextDisabled("%s", note);
}

} // namespace

DashboardPanel::DashboardPanel() {
    m_LastUpdateTime = std::chrono::high_resolution_clock::now();
    SZM::EventBus::GetInstance().Subscribe(
        "SimulationTick",
        [this](const std::any& /*payload*/) { this->UpdateMetrics(); }
    );
}

void DashboardPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Forge control room");
    ImGui::SameLine();
    SZM::UIStyleHelper::StatusBadge(m_DangerousComponentCount > 0 ? "warning" : "online");
    ImGui::TextDisabled("Watch simulation load, thermal drift, and system health from one place.");

    ImGui::Checkbox("Detailed metrics##dashboard", &m_ShowDetailedMetrics);
    ImGui::SameLine();
    ImGui::Checkbox("Trend graphs##dashboard", &m_ShowGraphs);
    ImGui::SameLine();
    ImGui::Checkbox("Warnings##dashboard", &m_ShowWarnings);
    SZM::UIStyleHelper::HorizontalSeparator();

    RenderMetricCards();
    ImGui::Spacing();

    if (ImGui::BeginTable("DashboardLayout", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        RenderPerformanceMetrics();
        ImGui::Spacing();
        RenderSystemHealth();

        ImGui::TableNextColumn();
        RenderComponentStatistics();
        ImGui::Spacing();
        RenderSimulationStatus();

        if (m_ShowWarnings) {
            ImGui::Spacing();
            RenderWarnings();
        }

        ImGui::EndTable();
    }

    if (m_ShowGraphs) {
        ImGui::Spacing();
        RenderTrendGraphs();
    }

    ImGui::End();
}

void DashboardPanel::RenderMetricCards() {
    char fpsValue[32];
    char componentValue[32];
    char riskValue[32];
    char healthValue[32];

    const float overallHealth = std::clamp(
        1.0f - std::max(m_AverageStress / 250e6f, m_AverageTempRatio),
        0.0f,
        1.0f
    );

    std::snprintf(fpsValue, sizeof(fpsValue), "%.0f", m_CurrentFps);
    std::snprintf(componentValue, sizeof(componentValue), "%zu", m_ComponentCount);
    std::snprintf(riskValue, sizeof(riskValue), "%zu", m_DangerousComponentCount);
    std::snprintf(healthValue, sizeof(healthValue), "%.0f%%", overallHealth * 100.0f);

    if (ImGui::BeginTable("DashboardMetrics", 4, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        BeginSurface("MetricFps", 92.0f);
        DrawBigMetric("Frame rate", fpsValue, "Live viewport responsiveness", SZM::UIStyleHelper::Colors::ForgeBlue500);
        EndSurface();

        ImGui::TableNextColumn();
        BeginSurface("MetricComponents", 92.0f);
        DrawBigMetric("Components", componentValue, "Tracked simulation parts", SZM::UIStyleHelper::Colors::Teal500);
        EndSurface();

        ImGui::TableNextColumn();
        BeginSurface("MetricRisk", 92.0f);
        DrawBigMetric("At risk", riskValue, "Components above safe limits", SZM::UIStyleHelper::Colors::Copper500);
        EndSurface();

        ImGui::TableNextColumn();
        BeginSurface("MetricHealth", 92.0f);
        DrawBigMetric("System health", healthValue, GetHealthStatus(overallHealth), GetHealthColor(overallHealth));
        EndSurface();
        ImGui::EndTable();
    }
}

void DashboardPanel::RenderPerformanceMetrics() {
    BeginSurface("PerformanceSurface", 0.0f);
    ImGui::Text("Performance lane");
    SZM::UIStyleHelper::HorizontalSeparator();

    ImGui::TextDisabled("Frame time");
    ImGui::Text("%.2f ms", m_CurrentFrameTime * 1000.0f);
    const float frameBudgetRatio = std::clamp(1.0f - (m_CurrentFrameTime / 0.0166f), 0.0f, 1.0f);
    SZM::UIStyleHelper::ProgressBar(frameBudgetRatio, ImVec2(-1.0f, 0.0f), frameBudgetRatio > 0.55f ? "Healthy" : "Heavy");

    if (!m_FpsHistory.empty()) {
        std::vector<float> fpsVec(m_FpsHistory.begin(), m_FpsHistory.end());
        ImGui::TextDisabled("Recent FPS");
        ImGui::PlotLines("##FpsPreview", fpsVec.data(), static_cast<int>(fpsVec.size()), 0, nullptr, 0.0f, 120.0f, ImVec2(-1.0f, 72.0f));
    }

    if (m_ShowDetailedMetrics && !m_FpsHistory.empty() && !m_FrameTimeHistory.empty()) {
        const float minFps = *std::min_element(m_FpsHistory.begin(), m_FpsHistory.end());
        const float maxFps = *std::max_element(m_FpsHistory.begin(), m_FpsHistory.end());
        const float avgFps = std::accumulate(m_FpsHistory.begin(), m_FpsHistory.end(), 0.0f) / static_cast<float>(m_FpsHistory.size());
        const float minDt = *std::min_element(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end());
        const float maxDt = *std::max_element(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end());
        const float avgDt = std::accumulate(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end(), 0.0f) / static_cast<float>(m_FrameTimeHistory.size());

        ImGui::Spacing();
        ImGui::TextDisabled("Min %.1f fps  |  Avg %.1f fps  |  Max %.1f fps", minFps, avgFps, maxFps);
        ImGui::TextDisabled("Min %.2f ms   |  Avg %.2f ms   |  Max %.2f ms", minDt * 1000.0f, avgDt * 1000.0f, maxDt * 1000.0f);
    }
    EndSurface();
}

void DashboardPanel::RenderComponentStatistics() {
    BeginSurface("ComponentStatsSurface", 0.0f);
    ImGui::Text("Load distribution");
    SZM::UIStyleHelper::HorizontalSeparator();

    ImGui::TextDisabled("Average stress");
    ImGui::Text("%.2f MPa", m_AverageStress / 1.0e6f);
    SZM::UIStyleHelper::ProgressBar(std::clamp(m_AverageStress / 250e6f, 0.0f, 1.0f), ImVec2(-1.0f, 0.0f));

    ImGui::TextDisabled("Thermal saturation");
    ImGui::Text("%.0f%%", m_AverageTempRatio * 100.0f);
    SZM::UIStyleHelper::ProgressBar(std::clamp(m_AverageTempRatio, 0.0f, 1.0f), ImVec2(-1.0f, 0.0f));

    if (m_ShowDetailedMetrics) {
        ImGui::Spacing();
        ImGui::TextDisabled("Max stress %.2f MPa", m_MaxStress / 1.0e6f);
        ImGui::TextDisabled("Max temperature %.0f%%", m_MaxTempRatio * 100.0f);
    }
    EndSurface();
}

void DashboardPanel::RenderSimulationStatus() {
    BeginSurface("SimulationStatusSurface", 0.0f);
    ImGui::Text("Simulation lane");
    SZM::UIStyleHelper::HorizontalSeparator();

    const auto& components = SZM::SimulationEngine::GetInstance().GetComponents();
    
    if (components.empty()) {
        ImGui::TextDisabled("No components are loaded yet.");
        ImGui::Spacing();
        ImGui::TextWrapped("Use the top toolbar or Simulation Control panel to add beams, brackets, or demo parts to the workspace.");
    } else {
        ImGui::TextDisabled("Active components: %zu", components.size());
        SZM::UIStyleHelper::StatusBadge(m_DangerousComponentCount > 0 ? "warning" : "online");
        
        // Show top 3 most stressed components
        std::vector<std::pair<float, std::string>> stressedComps;
        for (const auto& comp : components) {
            stressedComps.push_back({comp->stressRatio, comp->name});
        }
        std::sort(stressedComps.rbegin(), stressedComps.rend());
        
        if (!stressedComps.empty()) {
            ImGui::Spacing();
            ImGui::TextDisabled("Most stressed");
            for (size_t i = 0; i < std::min(size_t(3), stressedComps.size()); ++i) {
                const auto& [ratio, name] = stressedComps[i];
                const ImVec4 color = ratio > 0.8f
                    ? ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::Colors::Red500)
                    : ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::Colors::Copper500);
                ImGui::TextColored(color, "%zu. %s", i + 1, name.c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("(%.1f%% stress)", ratio * 100.0f);
            }
        }
    }
    EndSurface();
}

void DashboardPanel::RenderSystemHealth() {
    BeginSurface("SystemHealthSurface", 0.0f);
    ImGui::Text("System health");
    SZM::UIStyleHelper::HorizontalSeparator();

    // Overall health based on stress and temperature
    float overallHealth = 1.0f - std::max(m_AverageStress / 250e6f, m_AverageTempRatio);
    overallHealth = std::clamp(overallHealth, 0.0f, 1.0f);

    ImGui::TextDisabled("Overall");
    ImGui::Text("%s", GetHealthStatus(overallHealth));
    SZM::UIStyleHelper::ProgressBar(overallHealth, ImVec2(-1.0f, 0.0f), nullptr);

    ImGui::Spacing();

    // Individual health indicators
    float stressHealth = 1.0f - (m_AverageStress / 250e6f);
    stressHealth = std::clamp(stressHealth, 0.0f, 1.0f);
    ImGui::TextDisabled("Stress Health:");
    SZM::UIStyleHelper::ProgressBar(stressHealth, ImVec2(-1.0f, 0.0f), nullptr);

    float tempHealth = 1.0f - m_AverageTempRatio;
    tempHealth = std::clamp(tempHealth, 0.0f, 1.0f);
    ImGui::TextDisabled("Thermal Health:");
    SZM::UIStyleHelper::ProgressBar(tempHealth, ImVec2(-1.0f, 0.0f), nullptr);
    EndSurface();
}

void DashboardPanel::RenderTrendGraphs() {
    BeginSurface("TrendGraphSurface", 220.0f);
    ImGui::Text("Trend graphs");
    SZM::UIStyleHelper::HorizontalSeparator();

    // FPS graph
    if (!m_FpsHistory.empty()) {
        std::vector<float> fpsVec(m_FpsHistory.begin(), m_FpsHistory.end());
        ImGui::PlotLines("FPS##graph", fpsVec.data(), static_cast<int>(fpsVec.size()),
                        0, nullptr, 0.0f, 120.0f, ImVec2(-1, 56));
    }

    // Stress average graph
    if (!m_StressAverageHistory.empty()) {
        std::vector<float> stressVec(m_StressAverageHistory.begin(), m_StressAverageHistory.end());
        ImGui::PlotLines("Avg Stress (MPa)##graph", stressVec.data(), static_cast<int>(stressVec.size()),
                        0, nullptr, 0.0f, 250.0f, ImVec2(-1, 56));
    }

    // Temperature average graph
    if (!m_TempAverageHistory.empty()) {
        std::vector<float> tempVec(m_TempAverageHistory.begin(), m_TempAverageHistory.end());
        ImGui::PlotLines("Avg Temp Ratio##graph", tempVec.data(), static_cast<int>(tempVec.size()),
                        0, nullptr, 0.0f, 1.0f, ImVec2(-1, 56));
    }
    EndSurface();
}

void DashboardPanel::RenderWarnings() {
    BeginSurface("WarningSurface", 0.0f);
    ImGui::Text("Warnings and alerts");
    SZM::UIStyleHelper::HorizontalSeparator();

    const auto& components = SZM::SimulationEngine::GetInstance().GetComponents();
    
    if (m_DangerousComponentCount > 0) {
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::Colors::Red500),
            "%zu component(s) are in a danger state",
            m_DangerousComponentCount
        );
        
        for (const auto& comp : components) {
            if (comp->isDangerous) {
                ImGui::BulletText("%s: Stress %.1f%% | Temp %.1f%%",
                                 comp->name.c_str(),
                                 comp->stressRatio * 100.0f,
                                 comp->tempRatio * 100.0f);
            }
        }
    } else {
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::Colors::Teal500),
            "All systems nominal"
        );
    }

    // High stress warning
    if (m_AverageStress > 200e6f) {
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::Colors::Copper500),
            "High average stress detected"
        );
    }

    // High temperature warning
    if (m_AverageTempRatio > 0.7f) {
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::Colors::Copper500),
            "High average temperature detected"
        );
    }

    // Low FPS warning
    if (m_CurrentFps < 30.0f && m_CurrentFps > 0.0f) {
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(SZM::UIStyleHelper::Colors::Copper500),
            "Low frame rate (%.1f FPS)",
            m_CurrentFps
        );
    }
    EndSurface();
}

void DashboardPanel::UpdateMetrics() {
    auto now = std::chrono::high_resolution_clock::now();
    double deltaTime = std::chrono::duration<double>(now - m_LastUpdateTime).count();
    m_LastUpdateTime = now;

    m_CurrentFrameTime = deltaTime;
    m_CurrentFps = deltaTime > 0.0 ? 1.0f / static_cast<float>(deltaTime) : 0.0f;

    m_TimeSinceLastSample += deltaTime;
    if (m_TimeSinceLastSample >= SAMPLE_INTERVAL) {
        RecordMetricSample();
        m_TimeSinceLastSample = 0.0;
    }

    // Update component statistics
    const auto& components = SZM::SimulationEngine::GetInstance().GetComponents();
    m_ComponentCount = components.size();
    m_DangerousComponentCount = 0;
    m_AverageStress = 0.0f;
    m_AverageTempRatio = 0.0f;
    m_MaxStress = 0.0f;
    m_MaxTempRatio = 0.0f;

    if (!components.empty()) {
        for (const auto& comp : components) {
            if (comp->isDangerous) {
                ++m_DangerousComponentCount;
            }
            m_AverageStress += comp->stress;
            m_AverageTempRatio += comp->tempRatio;
            m_MaxStress = std::max(m_MaxStress, comp->stress);
            m_MaxTempRatio = std::max(m_MaxTempRatio, comp->tempRatio);
        }
        m_AverageStress /= components.size();
        m_AverageTempRatio /= components.size();
    }
}

void DashboardPanel::RecordMetricSample() {
    m_FpsHistory.push_back(m_CurrentFps);
    if (m_FpsHistory.size() > MAX_HISTORY) {
        m_FpsHistory.pop_front();
    }

    m_FrameTimeHistory.push_back(m_CurrentFrameTime);
    if (m_FrameTimeHistory.size() > MAX_HISTORY) {
        m_FrameTimeHistory.pop_front();
    }

    m_StressAverageHistory.push_back(m_AverageStress);
    if (m_StressAverageHistory.size() > MAX_HISTORY) {
        m_StressAverageHistory.pop_front();
    }

    m_TempAverageHistory.push_back(m_AverageTempRatio);
    if (m_TempAverageHistory.size() > MAX_HISTORY) {
        m_TempAverageHistory.pop_front();
    }
}

const char* DashboardPanel::GetHealthStatus(float ratio) const {
    if (ratio > 0.8f) return "Excellent";
    if (ratio > 0.6f) return "Good";
    if (ratio > 0.4f) return "Fair";
    if (ratio > 0.2f) return "Poor";
    return "Critical";
}

ImU32 DashboardPanel::GetHealthColor(float ratio) const {
    if (ratio > 0.8f) return SZM::UIStyleHelper::Colors::Teal500;
    if (ratio > 0.6f) return SZM::UIStyleHelper::Colors::Blue500;
    if (ratio > 0.4f) return SZM::UIStyleHelper::Colors::Copper500;
    if (ratio > 0.2f) return SZM::UIStyleHelper::Colors::Amber500;
    return SZM::UIStyleHelper::Colors::Red500;
}
