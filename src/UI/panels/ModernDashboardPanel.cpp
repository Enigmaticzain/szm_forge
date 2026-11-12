#include "ModernDashboardPanel.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include <algorithm>
#include <cstdio>
#include <imgui.h>
#include <string>

namespace SZM {

namespace {

struct DashboardSnapshot {
    size_t componentCount = 0;
    size_t dangerousCount = 0;
    float maxStressRatio = 0.0f;
    float avgStressRatio = 0.0f;
    float maxTempRatio = 0.0f;
    float healthScore = 1.0f;
    bool paused = false;
};

DashboardSnapshot CollectSnapshot() {
    auto& engine = SimulationEngine::GetInstance();
    DashboardSnapshot snap;
    snap.paused = engine.IsPaused();
    const auto& comps = engine.GetComponents();
    snap.componentCount = comps.size();
    if (comps.empty()) {
        return snap;
    }

    float stressSum = 0.0f;
    for (const auto& c : comps) {
        if (!c) continue;
        if (c->isDangerous) {
            snap.dangerousCount++;
        }
        snap.maxStressRatio = std::max(snap.maxStressRatio, std::clamp(c->stressRatio, 0.0f, 1.0f));
        stressSum += std::clamp(c->stressRatio, 0.0f, 1.0f);
        float tRatio = std::clamp((c->temperature - 293.15f) / 100.0f, 0.0f, 1.0f);
        snap.maxTempRatio = std::max(snap.maxTempRatio, tRatio);
    }
    snap.avgStressRatio = stressSum / static_cast<float>(comps.size());
    float dangerPenalty = static_cast<float>(snap.dangerousCount) / static_cast<float>(comps.size());
    snap.healthScore = std::clamp(1.0f - (0.5f * snap.maxStressRatio + 0.5f * dangerPenalty), 0.0f, 1.0f);
    return snap;
}

} // namespace

void ModernDashboardPanel::DrawHeader() {
    const auto snap = CollectSnapshot();
    ImGui::TextUnformatted("System Overview");
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    UIStyleHelper::StatusBadge(snap.paused ? "idle" : (snap.dangerousCount > 0 ? "warning" : "online"));
}

void ModernDashboardPanel::DrawContent() {
    DrawMetricsCards();
    ImGui::Spacing();
    ImGui::Spacing();
    
    ImGui::Columns(2, nullptr, true);
    DrawSystemHealth();
    ImGui::NextColumn();
    DrawRecentActivity();
    ImGui::Columns(1);
}

void ModernDashboardPanel::DrawMetricsCards() {
    const auto snap = CollectSnapshot();
    ImGui::Text("Key Metrics");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    ImGui::Columns(4, nullptr, false);
    
    // Card 1: Active Components
    {
        BeginCard();
        ImGui::TextUnformatted("Components");
        ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "%zu", snap.componentCount);
        ImGui::TextDisabled("%s", snap.paused ? "Paused" : "Active");
        EndCard();
    }
    ImGui::NextColumn();

    // Card 2: Structural Health
    {
        BeginCard();
        ImGui::TextUnformatted("System Health");
        ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "%.0f%%", snap.healthScore * 100.0f);
        ImGui::TextDisabled("%s", snap.healthScore > 0.8f ? "Optimal" : "Check Loads");
        EndCard();
    }
    ImGui::NextColumn();

    // Card 3: Peak Stress Ratio
    {
        BeginCard();
        ImGui::TextUnformatted("Peak Stress");
        ImGui::TextColored(ImVec4(0.23f, 0.51f, 0.96f, 1.0f), "%.1f%%", snap.maxStressRatio * 100.0f);
        ImGui::TextDisabled("Of Yield");
        EndCard();
    }
    ImGui::NextColumn();

    // Card 4: Danger Alerts
    {
        BeginCard();
        ImGui::TextUnformatted("Overloaded");
        ImGui::TextColored(snap.dangerousCount > 0 ? ImVec4(0.95f, 0.25f, 0.25f, 1.0f)
                                                   : ImVec4(0.96f, 0.62f, 0.04f, 1.0f),
                           "%zu", snap.dangerousCount);
        ImGui::TextDisabled("Flagged");
        EndCard();
    }

    ImGui::Columns(1);
}

void ModernDashboardPanel::DrawSystemHealth() {
    const auto snap = CollectSnapshot();
    ImGui::Text("System Health");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    struct HealthMetric {
        const char* name;
        float value;
        const char* status;
    };

    HealthMetric metrics[] = {
        {"Peak Stress", snap.maxStressRatio, snap.maxStressRatio > 0.8f ? "warning" : "active"},
        {"Avg Stress", snap.avgStressRatio, snap.avgStressRatio > 0.7f ? "warning" : "active"},
        {"Thermal Load", snap.maxTempRatio, snap.maxTempRatio > 0.8f ? "warning" : "active"},
        {"Overall Health", snap.healthScore, snap.healthScore > 0.75f ? "active" : "warning"}
    };

    for (const auto& metric : metrics) {
        ImGui::TextUnformatted(metric.name);
        ImGui::SameLine(150);
        UIStyleHelper::ProgressBar(metric.value, ImVec2(100, 0));
        ImGui::SameLine();
        UIStyleHelper::StatusBadge(metric.status);
        ImGui::Spacing();
    }
}

void ModernDashboardPanel::DrawRecentActivity() {
    const auto& comps = SimulationEngine::GetInstance().GetComponents();
    ImGui::Text("Component Status");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    if (comps.empty()) {
        ImGui::TextDisabled("No components registered in SimulationEngine.");
        return;
    }

    size_t shown = 0;
    for (auto it = comps.rbegin(); it != comps.rend() && shown < 5; ++it) {
        const auto& comp = *it;
        if (!comp) continue;
        char summary[64];
        std::snprintf(summary, sizeof(summary), "Stress %.0f%% | %.1f K",
                      comp->stressRatio * 100.0f, comp->temperature);
        ImGui::TextUnformatted(comp->name.c_str());
        ImGui::SameLine(120);
        ImGui::TextDisabled("%s", summary);
        ImGui::SameLine(ImGui::GetWindowWidth() - 80);
        UIStyleHelper::StatusBadge(comp->isDangerous ? "warning" : "active");
        ImGui::Spacing();
        ++shown;
    }
}

} // namespace SZM
