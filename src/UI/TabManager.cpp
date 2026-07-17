#include "TabManager.hpp"
#include "UIStyleHelper.hpp"
#include <algorithm>
#include <imgui_internal.h>
#include <cmath>

namespace SZM {

namespace {

float CalculateTabButtonWidth(const TabManager::Tab& tab, float availableWidth) {
    const std::string displayLabel = tab.icon.empty() ? tab.label : tab.icon + " " + tab.label;
    const float textWidth = ImGui::CalcTextSize(displayLabel.c_str()).x;
    const float maxWidth = std::max(availableWidth, 120.0f);
    return std::clamp(textWidth + 30.0f, 120.0f, std::min(196.0f, maxWidth));
}

} // namespace

void TabManager::AddTab(const std::string& id, const std::string& label, const std::string& icon, std::function<void()> content) {
    m_Tabs.push_back({id, label, icon, content, true});
    if (m_ActiveTab.empty()) {
        m_ActiveTab = id;
    }
}

void TabManager::RemoveTab(const std::string& id) {
    auto it = std::find_if(m_Tabs.begin(), m_Tabs.end(), [&id](const Tab& t) { return t.id == id; });
    if (it != m_Tabs.end()) {
        m_Tabs.erase(it);
        EnsureValidActiveTab();
    }
}

void TabManager::SetActiveTab(const std::string& id) {
    if (!HasVisibleTab(id)) {
        EnsureValidActiveTab();
        return;
    }

    if (m_ActiveTab != id) {
        m_PreviousTab = m_ActiveTab;
        m_ActiveTab = id;
        m_IsAnimating = true;
        m_AnimationProgress = 0.0f;
    }
}

bool TabManager::HasVisibleTab(const std::string& id) const {
    return std::find_if(
        m_Tabs.begin(),
        m_Tabs.end(),
        [&id](const Tab& tab) {
            return tab.visible && tab.id == id;
        }
    ) != m_Tabs.end();
}

void TabManager::EnsureValidActiveTab() {
    if (m_Tabs.empty()) {
        m_ActiveTab.clear();
        m_PreviousTab.clear();
        m_IsAnimating = false;
        m_AnimationProgress = 1.0f;
        return;
    }

    if (!HasVisibleTab(m_ActiveTab)) {
        const auto it = std::find_if(
            m_Tabs.begin(),
            m_Tabs.end(),
            [](const Tab& tab) {
                return tab.visible;
            }
        );
        m_ActiveTab = it != m_Tabs.end() ? it->id : std::string{};
    }
}

void TabManager::UpdateAnimation(float deltaTime) {
    if (m_IsAnimating) {
        m_AnimationProgress += deltaTime / m_AnimationDuration;
        if (m_AnimationProgress >= 1.0f) {
            m_AnimationProgress = 1.0f;
            m_IsAnimating = false;
        }
    }
}

void TabManager::DrawTabBar(float width) {
    EnsureValidActiveTab();
    if (m_Tabs.empty()) {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));

    const float availableWidth = width > 0.0f ? width : std::max(ImGui::GetContentRegionAvail().x, 120.0f);
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    float rowWidth = 0.0f;

    for (const auto& tab : m_Tabs) {
        if (!tab.visible) continue;

        const float buttonWidth = CalculateTabButtonWidth(tab, availableWidth);
        if (rowWidth > 0.0f && rowWidth + buttonWidth > availableWidth) {
            ImGui::NewLine();
            rowWidth = 0.0f;
        } else if (rowWidth > 0.0f) {
            ImGui::SameLine();
        }

        DrawTabBarButton(tab, tab.id == m_ActiveTab, buttonWidth);
        rowWidth += buttonWidth + spacing;
    }

    ImGui::PopStyleVar(2);
    ImGui::NewLine();
    UIStyleHelper::HorizontalSeparator();
}

void TabManager::DrawTabBarButton(const Tab& tab, bool isActive, float width) {
    ImGui::PushID(tab.id.c_str());

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

    const std::string label = tab.icon.empty() ? tab.label : tab.icon + " " + tab.label;
    if (ImGui::Button(label.c_str(), ImVec2(width, 0.0f))) {
        SetActiveTab(tab.id);
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
    ImGui::PopID();
}

void TabManager::DrawTabContent() {
    EnsureValidActiveTab();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, UIStyleHelper::GetSurfaceColor(0));
    ImGui::PushStyleColor(ImGuiCol_Border, UIStyleHelper::GetBorderColor());
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

    ImGui::BeginChild("##ModernTabContent", ImVec2(0.0f, 0.0f), true);
    auto it = std::find_if(m_Tabs.begin(), m_Tabs.end(), [this](const Tab& t) { return t.id == m_ActiveTab; });
    if (it != m_Tabs.end() && it->content) {
        // Fade in animation
        float alpha = std::min(1.0f, m_AnimationProgress * 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        it->content();
        ImGui::PopStyleVar();
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void AnimatedPanel::UpdateAnimation(float deltaTime) {
    m_TargetAlpha = m_Visible ? 1.0f : 0.0f;
    
    if (std::abs(m_Alpha - m_TargetAlpha) > 0.01f) {
        m_Alpha += (m_TargetAlpha - m_Alpha) * m_AnimationSpeed * deltaTime;
    } else {
        m_Alpha = m_TargetAlpha;
    }
}

void ModernTabPanel::Draw() {
    ImGui::SetNextWindowClass(nullptr);
    if (ImGui::Begin(GetTitle())) {
        m_TabManager.UpdateAnimation(m_DeltaTime);

        // Header
        ImGui::TextUnformatted(GetTitle());
        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
        UIStyleHelper::StatusBadge("active");
        ImGui::Spacing();
        UIStyleHelper::HorizontalSeparator();
        ImGui::Spacing();

        // Tab bar
        m_TabManager.DrawTabBar();
        ImGui::Spacing();

        // Tab content
        m_TabManager.DrawTabContent();

        // Footer
        DrawFooter();
    }
    ImGui::End();
}

void ModernTabPanel::AddTab(const std::string& id, const std::string& label, const std::string& icon, std::function<void()> content) {
    m_TabManager.AddTab(id, label, icon, content);
}

} // namespace SZM
