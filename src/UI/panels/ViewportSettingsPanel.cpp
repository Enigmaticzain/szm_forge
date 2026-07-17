#include "ViewportSettingsPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

ViewportSettingsPanel::ViewportSettingsPanel() = default;

ViewportSettingsPanel::~ViewportSettingsPanel() = default;

void ViewportSettingsPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen)) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawRenderSettings();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Grid Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawGridSettings();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Display Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawDisplaySettings();
    }

    ImGui::End();
}

void ViewportSettingsPanel::DrawRenderSettings() {
    ImGui::ColorEdit3("Background Color", &m_BackgroundColor.x);

    const char* antialiasing[] = {"None", "2x MSAA", "4x MSAA", "8x MSAA"};
    static int aaMode = 2;
    ImGui::Combo("Anti-aliasing", &aaMode, antialiasing, IM_ARRAYSIZE(antialiasing));

    static bool vsync = true;
    ImGui::Checkbox("V-Sync", &vsync);

    static bool shadows = true;
    ImGui::Checkbox("Shadows", &shadows);

    static bool bloom = false;
    ImGui::Checkbox("Bloom", &bloom);
}

void ViewportSettingsPanel::DrawGridSettings() {
    ImGui::Checkbox("Show Grid", &m_ShowGrid);

    if (m_ShowGrid) {
        ImGui::ColorEdit3("Grid Color", &m_GridColor.x);
        ImGui::SliderFloat("Grid Size", &m_GridSize, 0.1f, 10.0f);
        ImGui::SliderFloat("Grid Spacing", &m_GridSpacing, 0.01f, 1.0f);
    }
}

void ViewportSettingsPanel::DrawDisplaySettings() {
    ImGui::Checkbox("Show Axes", &m_ShowAxes);
    ImGui::Checkbox("Show Statistics", &m_ShowStats);

    static bool showNormals = false;
    ImGui::Checkbox("Show Normals", &showNormals);

    static bool showBounds = false;
    ImGui::Checkbox("Show Bounds", &showBounds);

    ImGui::Separator();
    ImGui::Text("Viewport Info:");
    ImGui::BulletText("Resolution: 1920x1080");
    ImGui::BulletText("FOV: 45°");
    ImGui::BulletText("Near Plane: 0.1");
    ImGui::BulletText("Far Plane: 1000.0");
}

} // namespace SZM::UI
