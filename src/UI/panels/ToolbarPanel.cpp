#include "ToolbarPanel.hpp"
#include "../UIStyleHelper.hpp"
#include "../UITooltipHelper.hpp"
#include "../WorkshopTooltips.hpp"
#include "../../Simulation/SimulationEngine.hpp"

namespace WT = SZM::WorkshopTooltips;
#include <imgui.h>
#include <vector>

ToolbarPanel::ToolbarPanel() : m_SimulationSpeed(1.0f) {}

void ToolbarPanel::Draw() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse;

    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible, flags)) {
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("Simulation controls");

    if (ImGui::BeginTable("ToolbarGrid", 4, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        ImGui::Text("Playback");
        DrawPlaybackControls();

        ImGui::TableNextColumn();
        ImGui::Text("Speed and view");
        DrawSpeedControl();
        DrawViewModes();

        ImGui::TableNextColumn();
        ImGui::Text("Quick add");
        DrawQuickAdd();

        ImGui::TableNextColumn();
        ImGui::Text("Status");
        DrawStatus();
        ImGui::EndTable();
    }

    ImGui::End();
}

void ToolbarPanel::DrawPlaybackControls() {
    ImGui::PushID("PlaybackControls");
    
    auto& engine = SZM::SimulationEngine::GetInstance();
    bool isRunning = !engine.IsPaused();

    const char* playLabel = isRunning ? "Pause" : "Play";
    if (SZM::UIStyleHelper::ModernButtonPrimary(playLabel, WT::ToolbarPlay, ImVec2(80.0f, 0.0f))) {
        engine.SetPaused(isRunning);
    }
    ImGui::SameLine();

    if (SZM::UIStyleHelper::ModernButtonSecondary("Reset", WT::ToolbarReset, ImVec2(80.0f, 0.0f))) {
        engine.SetPaused(true);
        m_SimulationSpeed = 1.0f;
        engine.SetTimeScale(1.0);
    }
    
    ImGui::PopID();
}

void ToolbarPanel::DrawSpeedControl() {
    ImGui::PushID("SpeedControl");
    ImGui::TextDisabled("Simulation speed");
    ImGui::SetNextItemWidth(120.0f);
    
    if (ImGui::SliderFloat("Speed##sim", &m_SimulationSpeed, 0.1f, 2.0f, "%.1fx")) {
        engine.SetTimeScale(static_cast<double>(m_SimulationSpeed));
    }
    SZM::UITooltip::Hover(WT::ToolbarSpeed);
    
    ImGui::PopID();
}

void ToolbarPanel::DrawViewModes() {
    ImGui::PushID("ViewModes");
    ImGui::TextDisabled("Overlay focus");
    
    if (ImGui::Checkbox("Stress##view", &m_ShowStressHeatmap)) {
        if (m_ShowStressHeatmap) m_ShowTemperatureHeatmap = false;
    }
    SZM::UITooltip::Hover(WT::ToolbarStressOverlay);
    ImGui::SameLine();

    if (ImGui::Checkbox("Temp##view", &m_ShowTemperatureHeatmap)) {
        if (m_ShowTemperatureHeatmap) m_ShowStressHeatmap = false;
    }
    SZM::UITooltip::Hover(WT::ToolbarTempOverlay);
    
    ImGui::PopID();
}

void ToolbarPanel::DrawQuickAdd() {
    ImGui::PushID("QuickAdd");
    
    auto& engine = SZM::SimulationEngine::GetInstance();
    
    if (SZM::UIStyleHelper::ModernButtonSecondary("Beam", WT::ToolbarQuickBeam, ImVec2(68.0f, 0.0f))) {
        engine.AddComponent("Beam", "MAT-STEEL-STRUCT", 0.01f, 0.05f);
    }
    ImGui::SameLine();

    if (SZM::UIStyleHelper::ModernButtonSecondary("Bracket", WT::ToolbarQuickBracket, ImVec2(78.0f, 0.0f))) {
        engine.AddComponent("Bracket", "MAT-AL-6061-T6", 0.008f, 0.01f);
    }
    ImGui::SameLine();
    
    if (SZM::UIStyleHelper::ModernButton("Clear", ImVec2(62.0f, 0.0f))) {
        std::vector<uint32_t> ids;
        ids.reserve(engine.GetComponents().size());
        for (const auto& comp : engine.GetComponents()) {
            ids.push_back(comp->id);
        }
        for (const auto id : ids) {
            engine.RemoveComponent(id);
        }
    }
    
    ImGui::PopID();
}

void ToolbarPanel::DrawStatus() {
    auto& engine = SZM::SimulationEngine::GetInstance();
    size_t componentCount = engine.GetComponents().size();
    size_t dangerCount = 0;
    for (const auto& component : engine.GetComponents()) {
        if (component && component->isDangerous) {
            ++dangerCount;
        }
    }

    SZM::UIStyleHelper::StatusBadge(!engine.IsPaused() ? "running" : "paused");
    ImGui::TextDisabled("Components: %zu", componentCount);
    ImGui::TextDisabled("At risk: %zu", dangerCount);
    ImGui::TextDisabled("Speed: %.1fx", m_SimulationSpeed);
}
