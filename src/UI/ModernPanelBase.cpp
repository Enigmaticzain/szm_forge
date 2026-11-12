#include "ModernPanelBase.hpp"
#include <imgui.h>

namespace SZM {

void ModernPanelBase::Draw() {
    if (!IsVisible) return;
    m_CardIndex = 0;

    ImGui::SetNextWindowClass(nullptr);
    if (ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        if (ShowHeader()) {
            DrawHeader();
            UIStyleHelper::HorizontalSeparator();
            ImGui::Spacing();
        }
        DrawContent();
    }
    ImGui::End();
}

void ModernPanelBase::BeginCard() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, UIStyleHelper::GetSurfaceColor(0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, UIStyleHelper::GetBorderColor());
    const std::string cardId = std::string(GetName()) + "##card_" + std::to_string(m_CardIndex++);
    ImGui::BeginChild(ImGui::GetID(cardId.c_str()), ImVec2(0, 0), true);
}

void ModernPanelBase::EndCard() {
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void ModernPanelBase::DrawStatusIndicator(const std::string& status) {
    ImU32 color = UIStyleHelper::GetStatusColor(status);
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::SmallButton(status.c_str());
    ImGui::PopStyleColor();
}

void ModernPanelBase::DrawProgressBar(float progress, const char* label) {
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, UIStyleHelper::Colors::Violet500);
    ImGui::ProgressBar(progress, ImVec2(-1, 0), label);
    ImGui::PopStyleColor();
}

} // namespace SZM
