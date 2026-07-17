#include "EasyTabBar.hpp"
#include "UIStyleHelper.hpp"
#include <algorithm>

namespace SZM {

namespace {

float CalculateTabButtonWidth(const SimpleTab& tab, float availableWidth) {
    const std::string displayLabel = tab.icon.empty() ? tab.label : tab.icon + " " + tab.label;
    const float textWidth = ImGui::CalcTextSize(displayLabel.c_str()).x;
    const float maxWidth = std::max(availableWidth, 110.0f);
    return std::clamp(textWidth + 28.0f, 110.0f, std::min(188.0f, maxWidth));
}

} // namespace

void EasyTabBar::Draw() {
    EnsureValidActiveTab();
    if (m_Tabs.empty()) {
        return;
    }

    DrawTabButtons();
    DrawTabContent();
}

bool EasyTabBar::HasTab(const std::string& id) const {
    return std::find_if(
        m_Tabs.begin(),
        m_Tabs.end(),
        [&id](const SimpleTab& tab) {
            return tab.id == id;
        }
    ) != m_Tabs.end();
}

void EasyTabBar::SetActiveTab(const std::string& id) {
    if (HasTab(id)) {
        m_ActiveTabId = id;
        return;
    }

    EnsureValidActiveTab();
}

void EasyTabBar::EnsureValidActiveTab() {
    if (m_Tabs.empty()) {
        m_ActiveTabId.clear();
        return;
    }

    if (!HasTab(m_ActiveTabId)) {
        m_ActiveTabId = m_Tabs.front().id;
    }
}

void EasyTabBar::DrawTabButtons() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));

    const float availableWidth = std::max(ImGui::GetContentRegionAvail().x, 110.0f);
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    float rowWidth = 0.0f;

    for (const auto& tab : m_Tabs) {
        bool isActive = (tab.id == m_ActiveTabId);
        const float buttonWidth = CalculateTabButtonWidth(tab, availableWidth);

        if (rowWidth > 0.0f && rowWidth + buttonWidth > availableWidth) {
            ImGui::NewLine();
            rowWidth = 0.0f;
        } else if (rowWidth > 0.0f) {
            ImGui::SameLine();
        }

        // Active tab styling
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, UIStyleHelper::Colors::Violet500);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UIStyleHelper::Colors::Violet600);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, UIStyleHelper::Colors::Violet600);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, UIStyleHelper::GetSecondaryButtonColor());
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UIStyleHelper::GetSecondaryButtonHoverColor());
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, UIStyleHelper::GetSecondaryButtonActiveColor());
            ImGui::PushStyleColor(ImGuiCol_Text, UIStyleHelper::GetTextColor());
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        const std::string buttonLabel = tab.icon.empty() ? tab.label : tab.icon + " " + tab.label;
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(buttonWidth, 0.0f))) {
            m_ActiveTabId = tab.id;
        }
        if (!tab.tooltip.empty()) {
            ImGui::SetItemTooltip("%s", tab.tooltip.c_str());
        }

        if (isActive) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImVec2 min = ImGui::GetItemRectMin();
            const ImVec2 max = ImGui::GetItemRectMax();
            drawList->AddLine(
                ImVec2(min.x + 12.0f, max.y - 1.0f),
                ImVec2(max.x - 12.0f, max.y - 1.0f),
                UIStyleHelper::Colors::Copper500,
                3.0f
            );
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        rowWidth += buttonWidth + spacing;
    }

    ImGui::PopStyleVar(2);
    ImGui::NewLine();
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();
}

void EasyTabBar::DrawTabContent() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, UIStyleHelper::GetSurfaceColor(0));
    ImGui::PushStyleColor(ImGuiCol_Border, UIStyleHelper::GetBorderColor());
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

    ImGui::BeginChild("##EasyTabContent", ImVec2(0.0f, 0.0f), true);
    for (const auto& tab : m_Tabs) {
        if (tab.id == m_ActiveTabId && tab.draw) {
            tab.draw();
            break;
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

} // namespace SZM
