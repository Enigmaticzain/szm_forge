#include "TemplatePanel.hpp"
#include <imgui.h>

TemplatePanel::TemplatePanel() = default;

void TemplatePanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    DrawHeader();
    ImGui::Separator();
    DrawContent();
    ImGui::Separator();
    DrawFooter();

    ImGui::End();
}

void TemplatePanel::DrawHeader() {
    ImGui::Text("Panel Header");
    ImGui::SameLine();
    if (ImGui::Button("Details")) {
        m_ShowDetails = !m_ShowDetails;
    }
}

void TemplatePanel::DrawContent() {
    ImGui::SliderFloat("Value", &m_Value, 0.0f, 1.0f);

    if (m_ShowDetails) {
        ImGui::Indent();
        ImGui::Text("Detailed Information:");
        ImGui::Text("Value: %.2f", m_Value);
        ImGui::Unindent();
    }
}

void TemplatePanel::DrawFooter() {
    ImGui::Text("Status: OK");
}
