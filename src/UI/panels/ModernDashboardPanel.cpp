#include "ModernDashboardPanel.hpp"
#include <imgui.h>

namespace SZM {

void ModernDashboardPanel::DrawHeader() {
    ImGui::TextUnformatted("System Overview");
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    UIStyleHelper::StatusBadge("online");
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
    ImGui::Text("Key Metrics");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    ImGui::Columns(4, nullptr, false);
    
    // Card 1: Active Simulations
    {
        BeginCard();
        ImGui::TextUnformatted("Active Simulations");
        ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "12");
        ImGui::TextDisabled("Running");
        EndCard();
    }
    ImGui::NextColumn();

    // Card 2: System Health
    {
        BeginCard();
        ImGui::TextUnformatted("System Health");
        ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "94%%");
        ImGui::TextDisabled("Optimal");
        EndCard();
    }
    ImGui::NextColumn();

    // Card 3: Memory Usage
    {
        BeginCard();
        ImGui::TextUnformatted("Memory Usage");
        ImGui::TextColored(ImVec4(0.23f, 0.51f, 0.96f, 1.0f), "45/64 GB");
        ImGui::TextDisabled("70%% Used");
        EndCard();
    }
    ImGui::NextColumn();

    // Card 4: Last Update
    {
        BeginCard();
        ImGui::TextUnformatted("Last Update");
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "2 min");
        ImGui::TextDisabled("Ago");
        EndCard();
    }

    ImGui::Columns(1);
}

void ModernDashboardPanel::DrawSystemHealth() {
    ImGui::Text("System Health");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    struct HealthMetric {
        const char* name;
        float value;
        const char* status;
    };

    HealthMetric metrics[] = {
        {"CPU Load", 0.62f, "active"},
        {"GPU Load", 0.78f, "active"},
        {"Thermal", 0.45f, "active"},
        {"Memory", 0.70f, "warning"}
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
    ImGui::Text("Recent Activity");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    struct Activity {
        const char* type;
        const char* message;
        const char* status;
    };

    Activity activities[] = {
        {"Simulation", "Thermal analysis completed", "complete"},
        {"System", "Memory optimization running", "processing"},
        {"Analysis", "FEA solver initialized", "active"},
        {"Warning", "High thermal load detected", "warning"}
    };

    for (const auto& activity : activities) {
        ImGui::TextUnformatted(activity.type);
        ImGui::SameLine(100);
        ImGui::TextDisabled("%s", activity.message);
        ImGui::SameLine(ImGui::GetWindowWidth() - 80);
        UIStyleHelper::StatusBadge(activity.status);
        ImGui::Spacing();
    }
}

} // namespace SZM
