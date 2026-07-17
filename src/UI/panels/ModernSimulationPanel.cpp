#include "ModernSimulationPanel.hpp"
#include <imgui.h>

namespace SZM {

void ModernSimulationPanel::DrawHeader() {
    ImGui::TextUnformatted("Simulation Queue");
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    UIStyleHelper::StatusBadge("active");
}

void ModernSimulationPanel::DrawContent() {
    DrawSimulationControls();
    ImGui::Spacing();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
    
    DrawTaskQueue();
    ImGui::Spacing();
    
    DrawProgressMetrics();
}

void ModernSimulationPanel::DrawSimulationControls() {
    ImGui::Text("Controls");
    ImGui::Spacing();
    
    ImGui::Columns(3, nullptr, false);
    
    if (UIStyleHelper::ModernButtonPrimary("Start Simulation")) {
        // Start simulation
    }
    ImGui::NextColumn();
    
    if (UIStyleHelper::ModernButtonSecondary("Pause")) {
        // Pause simulation
    }
    ImGui::NextColumn();
    
    if (UIStyleHelper::ModernButtonSecondary("Stop")) {
        // Stop simulation
    }
    
    ImGui::Columns(1);
}

void ModernSimulationPanel::DrawTaskQueue() {
    ImGui::Text("Task Queue");
    ImGui::Spacing();
    
    for (auto& task : m_Tasks) {
        ImGui::PushID(task.name.c_str());
        
        BeginCard();
        
        // Task header
        ImGui::TextUnformatted(task.name.c_str());
        ImGui::SameLine(200);
        UIStyleHelper::StatusBadge(task.status);
        
        ImGui::Spacing();
        
        // Task type
        ImGui::TextDisabled("%s", ("Type: " + task.type).c_str());
        
        // Progress bar
        ImGui::Spacing();
        DrawProgressBar(task.progress, ("Progress: " + std::to_string(static_cast<int>(task.progress * 100)) + "%").c_str());
        
        EndCard();
        ImGui::PopID();
        ImGui::Spacing();
    }
}

void ModernSimulationPanel::DrawProgressMetrics() {
    ImGui::Text("Overall Progress");
    ImGui::Spacing();
    
    float totalProgress = 0.0f;
    for (const auto& task : m_Tasks) {
        totalProgress += task.progress;
    }
    totalProgress /= m_Tasks.size();
    
    ImGui::Columns(2, nullptr, false);
    
    ImGui::Text("Completed Tasks");
    ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "1/3");
    
    ImGui::NextColumn();
    
    ImGui::Text("Average Progress");
    ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "%.0f%%", totalProgress * 100);
    
    ImGui::Columns(1);
}

} // namespace SZM
