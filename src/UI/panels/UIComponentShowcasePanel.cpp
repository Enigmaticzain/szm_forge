#include "UIComponentShowcasePanel.hpp"
#include <imgui.h>

namespace SZM {

void UIComponentShowcasePanel::DrawContent() {
    if (ImGui::BeginTabBar("##showcase_tabs")) {
        if (ImGui::TabItemButton("Colors", 0)) {
            ShowColors();
        }
        if (ImGui::TabItemButton("Buttons", 0)) {
            ShowButtons();
        }
        if (ImGui::TabItemButton("Status Badges", 0)) {
            ShowStatusBadges();
        }
        if (ImGui::TabItemButton("Progress", 0)) {
            ShowProgressBars();
        }
        if (ImGui::TabItemButton("Cards", 0)) {
            ShowCards();
        }
        ImGui::EndTabBar();
    }
}

void UIComponentShowcasePanel::ShowColors() {
    ImGui::Text("Color Palette");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    struct ColorSwatch {
        const char* name;
        ImU32 color;
    };

    ColorSwatch colors[] = {
        {"Slate50", UIStyleHelper::Colors::Slate50},
        {"Slate100", UIStyleHelper::Colors::Slate100},
        {"Slate200", UIStyleHelper::Colors::Slate200},
        {"Slate500", UIStyleHelper::Colors::Slate500},
        {"Violet500", UIStyleHelper::Colors::Violet500},
        {"Emerald500", UIStyleHelper::Colors::Emerald500},
        {"Amber500", UIStyleHelper::Colors::Amber500},
        {"Red500", UIStyleHelper::Colors::Red500},
        {"Blue500", UIStyleHelper::Colors::Blue500},
    };

    ImGui::Columns(3, nullptr, false);
    for (const auto& swatch : colors) {
        ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        col.x = ((swatch.color >> 0) & 0xFF) / 255.0f;
        col.y = ((swatch.color >> 8) & 0xFF) / 255.0f;
        col.z = ((swatch.color >> 16) & 0xFF) / 255.0f;
        col.w = ((swatch.color >> 24) & 0xFF) / 255.0f;
        ImGui::ColorButton(swatch.name, col, ImGuiColorEditFlags_NoTooltip, ImVec2(60, 40));
        ImGui::SameLine();
        ImGui::TextUnformatted(swatch.name);
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

void UIComponentShowcasePanel::ShowButtons() {
    ImGui::Text("Button Variants");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    ImGui::TextUnformatted("Primary Buttons:");
    UIStyleHelper::ModernButtonPrimary("Primary Button");
    ImGui::SameLine();
    UIStyleHelper::ModernButtonPrimary("Another Primary");

    ImGui::Spacing();
    ImGui::TextUnformatted("Secondary Buttons:");
    UIStyleHelper::ModernButtonSecondary("Secondary Button");
    ImGui::SameLine();
    UIStyleHelper::ModernButtonSecondary("Another Secondary");

    ImGui::Spacing();
    ImGui::TextUnformatted("Default Buttons:");
    UIStyleHelper::ModernButton("Default Button");
    ImGui::SameLine();
    UIStyleHelper::ModernButton("Another Default");
}

void UIComponentShowcasePanel::ShowStatusBadges() {
    ImGui::Text("Status Badges");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    const char* statuses[] = {
        "active", "online", "complete",
        "processing", "running", "initializing",
        "idle", "pending",
        "offline", "error", "failed",
        "warning"
    };

    for (const auto* status : statuses) {
        UIStyleHelper::StatusBadge(status);
        ImGui::SameLine();
    }
    ImGui::NewLine();
}

void UIComponentShowcasePanel::ShowProgressBars() {
    ImGui::Text("Progress Bars");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    static float progress1 = 0.25f;
    static float progress2 = 0.50f;
    static float progress3 = 0.75f;
    static float progress4 = 1.00f;

    ImGui::TextUnformatted("25% Progress:");
    UIStyleHelper::ProgressBar(progress1, ImVec2(-1, 0), "25%");

    ImGui::TextUnformatted("50% Progress:");
    UIStyleHelper::ProgressBar(progress2, ImVec2(-1, 0), "50%");

    ImGui::TextUnformatted("75% Progress:");
    UIStyleHelper::ProgressBar(progress3, ImVec2(-1, 0), "75%");

    ImGui::TextUnformatted("100% Progress:");
    UIStyleHelper::ProgressBar(progress4, ImVec2(-1, 0), "Complete");
}

void UIComponentShowcasePanel::ShowCards() {
    ImGui::Text("Card Components");
    UIStyleHelper::HorizontalSeparator();
    ImGui::Spacing();

    ImGui::Columns(2, nullptr, true);

    BeginCard();
    ImGui::TextUnformatted("Card 1");
    ImGui::TextDisabled("This is a card component");
    ImGui::Spacing();
    UIStyleHelper::StatusBadge("active");
    EndCard();

    ImGui::NextColumn();

    BeginCard();
    ImGui::TextUnformatted("Card 2");
    ImGui::TextDisabled("Another card example");
    ImGui::Spacing();
    UIStyleHelper::ProgressBar(0.6f, ImVec2(-1, 0), "60%");
    EndCard();

    ImGui::Columns(1);
}

} // namespace SZM
