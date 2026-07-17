#include "HeatmapLegendPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

HeatmapLegendPanel::HeatmapLegendPanel()
    : m_Heatmap(Graphics::Heatmap::Mode::Stress) {
}

HeatmapLegendPanel::~HeatmapLegendPanel() = default;

void HeatmapLegendPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen)) {
        ImGui::End();
        return;
    }

    DrawColorScale();
    ImGui::Separator();
    DrawLegendInfo();

    ImGui::End();
}

void HeatmapLegendPanel::DrawColorScale() {
    ImGui::Text("Color Scale");
    ImGui::Separator();

    const int numSteps = 256;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize(ImGui::GetContentRegionAvail().x, 30);

    for (int i = 0; i < numSteps; ++i) {
        float ratio = static_cast<float>(i) / numSteps;
        auto color = m_Heatmap.GetColor(ratio);

        ImU32 imguiColor = ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, 1.0f));
        float x = canvasPos.x + (canvasSize.x * i) / numSteps;
        float y = canvasPos.y;

        drawList->AddRectFilled(
            ImVec2(x, y),
            ImVec2(x + canvasSize.x / numSteps + 1, y + canvasSize.y),
            imguiColor
        );
    }

    ImGui::Dummy(canvasSize);

    ImGui::Text("0.0");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30);
    ImGui::Text("1.0");
}

void HeatmapLegendPanel::DrawLegendInfo() {
    ImGui::Text("Legend Information:");
    ImGui::Indent();

    switch (m_Heatmap.GetMode()) {
        case Graphics::Heatmap::Mode::Stress:
            ImGui::BulletText("Blue: Low stress (safe)");
            ImGui::BulletText("Yellow: Medium stress");
            ImGui::BulletText("Red: High stress (critical)");
            break;
        case Graphics::Heatmap::Mode::Temperature:
            ImGui::BulletText("Blue: Cold regions");
            ImGui::BulletText("Purple: Moderate temperature");
            ImGui::BulletText("Red: Hot regions");
            break;
        case Graphics::Heatmap::Mode::DangerIndicator:
            ImGui::BulletText("Green: Safe");
            ImGui::BulletText("Red: Dangerous");
            break;
        case Graphics::Heatmap::Mode::None:
            ImGui::BulletText("No heatmap active");
            break;
    }

    ImGui::Unindent();
}

} // namespace SZM::UI
