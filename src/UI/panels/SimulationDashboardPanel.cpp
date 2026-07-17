#include "SimulationDashboardPanel.hpp"
#include "../UIStyleHelper.hpp"
#include <imgui.h>

namespace SZM {

SimulationDashboardPanel::SimulationDashboardPanel() {
    AddTab("overview", "Overview", "📊", [this]() { DrawOverviewTab(); });
    AddTab("tasks", "Tasks", "✓", [this]() { DrawTasksTab(); });
    AddTab("analytics", "Analytics", "📈", [this]() { DrawAnalyticsTab(); });
    AddTab("settings", "Settings", "⚙️", [this]() { DrawSettingsTab(); });
}

void SimulationDashboardPanel::DrawOverviewTab() {
    ImGui::Columns(2, nullptr, true);

    // Left column - Status
    ImGui::TextUnformatted("System Status");
    ImGui::Spacing();
    
    ImGui::TextUnformatted("Active Simulations:");
    ImGui::SameLine(200);
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "12");
    
    ImGui::TextUnformatted("Completed:");
    ImGui::SameLine(200);
    ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "847");
    
    ImGui::TextUnformatted("Failed:");
    ImGui::SameLine(200);
    ImGui::TextColored(ImVec4(0.93f, 0.27f, 0.27f, 1.0f), "3");

    ImGui::NextColumn();

    // Right column - Resources
    ImGui::TextUnformatted("Resource Usage");
    ImGui::Spacing();
    
    ImGui::TextUnformatted("CPU:");
    ImGui::SameLine(200);
    ImGui::ProgressBar(0.62f, ImVec2(100, 0), "62%");
    
    ImGui::TextUnformatted("GPU:");
    ImGui::SameLine(200);
    ImGui::ProgressBar(0.78f, ImVec2(100, 0), "78%");
    
    ImGui::TextUnformatted("Memory:");
    ImGui::SameLine(200);
    ImGui::ProgressBar(0.45f, ImVec2(100, 0), "45%");

    ImGui::Columns(1);
}

void SimulationDashboardPanel::DrawTasksTab() {
    ImGui::TextUnformatted("Active Tasks");
    ImGui::Spacing();

    struct Task {
        const char* name;
        const char* type;
        float progress;
        const char* status;
    };

    Task tasks[] = {
        {"Thermal Analysis", "thermal", 0.67f, "running"},
        {"Stress Test", "structural", 1.0f, "complete"},
        {"CFD Simulation", "fluid", 0.0f, "pending"},
        {"Vibration Analysis", "mechanical", 0.34f, "running"}
    };

    for (const auto& task : tasks) {
        ImGui::PushID(task.name);
        
        ImGui::TextUnformatted(task.name);
        ImGui::SameLine(200);
        UIStyleHelper::StatusBadge(task.status);
        
        ImGui::ProgressBar(task.progress, ImVec2(-1, 0), "");
        ImGui::Spacing();
        
        ImGui::PopID();
    }
}

void SimulationDashboardPanel::DrawAnalyticsTab() {
    ImGui::TextUnformatted("Performance Metrics");
    ImGui::Spacing();

    ImGui::Columns(2, nullptr, true);

    ImGui::TextUnformatted("Average Accuracy:");
    ImGui::SameLine(200);
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "94.2%%");

    ImGui::TextUnformatted("Avg Execution Time:");
    ImGui::SameLine(200);
    ImGui::TextColored(ImVec4(0.23f, 0.51f, 0.96f, 1.0f), "2.3s");

    ImGui::NextColumn();

    ImGui::TextUnformatted("Success Rate:");
    ImGui::SameLine(200);
    ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "99.6%%");

    ImGui::TextUnformatted("Total Simulations:");
    ImGui::SameLine(200);
    ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "850");

    ImGui::Columns(1);
}

void SimulationDashboardPanel::DrawSettingsTab() {
    ImGui::TextUnformatted("Simulation Settings");
    ImGui::Spacing();

    static int meshQuality = 2;
    ImGui::SliderInt("Mesh Quality", &meshQuality, 1, 5);

    static bool enableFEA = true;
    ImGui::Checkbox("Enable FEA Analysis", &enableFEA);

    static bool enableThermal = true;
    ImGui::Checkbox("Enable Thermal Analysis", &enableThermal);

    static float timeStep = 0.01f;
    ImGui::DragFloat("Time Step", &timeStep, 0.001f, 0.001f, 0.1f);

    ImGui::Spacing();
    if (UIStyleHelper::ModernButtonPrimary("Save Settings")) {
        // Save settings
    }
}

} // namespace SZM
