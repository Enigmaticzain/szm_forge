#include "ViewportPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

ViewportPanel::ViewportPanel()
    : m_Viewport(std::make_unique<Graphics::Viewport>(1280, 720)),
      m_VisController(Graphics::VisualizationController::GetInstance()) {
}

ViewportPanel::~ViewportPanel() = default;

void ViewportPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen, ImGuiWindowFlags_NoMove)) {
        ImGui::End();
        return;
    }

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        m_Viewport->Resize(static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y));
        ImGui::Image(
            static_cast<ImTextureID>(m_Viewport->GetTextureID()),
            viewportSize,
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
    }

    ImGui::Separator();
    DrawVisualizationControls();

    ImGui::End();
}

void ViewportPanel::DrawVisualizationControls() {
    if (ImGui::CollapsingHeader("Visualization Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawRenderModeSelector();
        ImGui::Separator();
        DrawLightingControls();
        ImGui::Separator();
        DrawHeatmapSelector();
    }
}

void ViewportPanel::DrawRenderModeSelector() {
    ImGui::Text("Render Mode:");
    ImGui::SameLine();

    const char* modes[] = {"Solid", "Wireframe", "Wireframe+Shaded", "Flat", "Normal Map", "Depth Map"};
    int currentMode = static_cast<int>(m_VisController.GetRenderMode());

    if (ImGui::Combo("##RenderMode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
        m_VisController.SetRenderMode(static_cast<Graphics::RenderMode>(currentMode));
    }

    if (ImGui::Checkbox("Show Wireframe Overlay", &m_IsOpen)) {
        m_VisController.ToggleWireframe();
    }
}

void ViewportPanel::DrawLightingControls() {
    ImGui::Text("Lighting:");
    ImGui::SameLine();

    const char* lightingModes[] = {"PBR", "Phong", "Flat"};
    int currentLighting = static_cast<int>(m_VisController.GetLightingMode());

    if (ImGui::Combo("##LightingMode", &currentLighting, lightingModes, IM_ARRAYSIZE(lightingModes))) {
        m_VisController.SetLightingMode(static_cast<Graphics::LightingMode>(currentLighting));
    }

    if (ImGui::Checkbox("Enable Lighting", &m_IsOpen)) {
        m_VisController.ToggleLighting();
    }

    float ambient = m_VisController.GetAmbientStrength();
    if (ImGui::SliderFloat("Ambient Strength", &ambient, 0.0f, 1.0f)) {
        m_VisController.SetAmbientStrength(ambient);
    }
}

void ViewportPanel::DrawHeatmapSelector() {
    ImGui::Text("Heatmap Mode:");
    ImGui::SameLine();

    const char* heatmapModes[] = {"Stress", "Temperature", "Danger", "None"};
    int currentHeatmap = static_cast<int>(m_VisController.GetHeatmapMode());

    if (ImGui::Combo("##HeatmapMode", &currentHeatmap, heatmapModes, IM_ARRAYSIZE(heatmapModes))) {
        m_VisController.SetHeatmapMode(static_cast<Graphics::Heatmap::Mode>(currentHeatmap));
    }
}

} // namespace SZM::UI
