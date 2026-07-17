#include "UIStyleHelper.hpp"
#include "UITooltipHelper.hpp"
#include <imgui_internal.h>

namespace SZM {

namespace {

UIStyleHelper::ThemeMode g_ThemeMode = UIStyleHelper::ThemeMode::Light;

ImVec4 ToVec4(ImU32 color) {
    return ImGui::ColorConvertU32ToFloat4(color);
}

void ApplySharedStyle(ImGuiStyle& style) {
    style.WindowRounding = 12.0f;
    style.FrameRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.TabRounding = 9.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.CellPadding = ImVec2(10.0f, 8.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 16.0f;
}

} // namespace

void UIStyleHelper::ApplyTheme(ThemeMode theme) {
    g_ThemeMode = theme;

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    if (theme == ThemeMode::Dark) {
        colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.10f, 0.13f, 1.0f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.14f, 0.18f, 1.0f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.14f, 0.18f, 0.98f);

        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.26f, 0.33f, 1.0f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.16f, 0.21f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.21f, 0.27f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.21f, 0.27f, 0.34f, 1.0f);

        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.11f, 0.15f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.15f, 0.20f, 1.0f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.11f, 0.15f, 0.80f);

        colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.12f, 0.16f, 1.0f);

        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.08f, 0.11f, 1.0f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.29f, 0.37f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.29f, 0.37f, 0.46f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.36f, 0.45f, 0.55f, 1.0f);

        colors[ImGuiCol_Button] = ToVec4(GetSecondaryButtonColor());
        colors[ImGuiCol_ButtonHovered] = ToVec4(GetSecondaryButtonHoverColor());
        colors[ImGuiCol_ButtonActive] = ToVec4(GetSecondaryButtonActiveColor());

        colors[ImGuiCol_Header] = ImVec4(0.15f, 0.20f, 0.26f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.26f, 0.33f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.32f, 0.40f, 1.0f);

        colors[ImGuiCol_Separator] = ToVec4(GetBorderColor());
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.36f, 0.45f, 0.55f, 1.0f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.46f, 0.56f, 0.67f, 1.0f);

        colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.36f, 0.44f, 0.55f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.36f, 0.45f, 0.55f, 0.85f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.56f, 0.67f, 1.0f);

        colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.16f, 0.21f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.16f, 0.21f, 0.27f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.24f, 0.31f, 1.0f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.09f, 0.12f, 0.16f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.19f, 0.25f, 1.0f);

        colors[ImGuiCol_Text] = ToVec4(GetTextColor());
        colors[ImGuiCol_TextDisabled] = ToVec4(GetMutedTextColor());

        colors[ImGuiCol_CheckMark] = ImVec4(0.18f, 0.70f, 0.66f, 1.0f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.36f, 0.54f, 0.74f, 1.0f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.18f, 0.70f, 0.66f, 1.0f);

        colors[ImGuiCol_PlotLines] = ImVec4(0.18f, 0.70f, 0.66f, 1.0f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.36f, 0.54f, 0.74f, 1.0f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.36f, 0.54f, 0.74f, 1.0f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.85f, 0.52f, 0.20f, 1.0f);

        colors[ImGuiCol_DockingPreview] = ImVec4(0.18f, 0.70f, 0.66f, 0.35f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.11f, 0.15f, 1.0f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.11f, 0.15f, 0.20f, 1.0f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.22f, 0.29f, 0.37f, 1.0f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.18f, 0.24f, 0.31f, 1.0f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.10f, 0.14f, 0.18f, 0.60f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.36f, 0.54f, 0.74f, 1.0f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.02f, 0.04f, 0.06f, 0.45f);
    } else {
        colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.92f, 0.89f, 1.0f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.985f, 0.98f, 0.965f, 1.0f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.985f, 0.98f, 0.965f, 1.0f);

        colors[ImGuiCol_Border] = ImVec4(0.86f, 0.82f, 0.76f, 1.0f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        colors[ImGuiCol_FrameBg] = ImVec4(0.96f, 0.945f, 0.925f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.93f, 0.90f, 0.86f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.86f, 0.80f, 1.0f);

        colors[ImGuiCol_TitleBg] = ImVec4(0.935f, 0.90f, 0.86f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.96f, 0.945f, 0.925f, 1.0f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.94f, 0.92f, 0.89f, 0.75f);

        colors[ImGuiCol_MenuBarBg] = ImVec4(0.93f, 0.90f, 0.86f, 1.0f);

        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.95f, 0.92f, 0.89f, 1.0f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.84f, 0.80f, 0.74f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.74f, 0.70f, 0.65f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.62f, 0.58f, 0.54f, 1.0f);

        colors[ImGuiCol_Button] = ImVec4(0.90f, 0.87f, 0.83f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.84f, 0.80f, 0.75f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.74f, 0.69f, 1.0f);

        colors[ImGuiCol_Header] = ImVec4(0.90f, 0.87f, 0.83f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.84f, 0.80f, 0.75f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.78f, 0.74f, 0.69f, 1.0f);

        colors[ImGuiCol_Separator] = ImVec4(0.84f, 0.80f, 0.74f, 1.0f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.66f, 0.60f, 1.0f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.60f, 0.54f, 0.49f, 1.0f);

        colors[ImGuiCol_ResizeGrip] = ImVec4(0.84f, 0.80f, 0.74f, 0.55f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.72f, 0.66f, 0.60f, 0.85f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.60f, 0.54f, 0.49f, 1.0f);

        colors[ImGuiCol_Tab] = ImVec4(0.90f, 0.87f, 0.83f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.84f, 0.80f, 0.75f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.98f, 0.97f, 0.955f, 1.0f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.94f, 0.92f, 0.89f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.95f, 0.94f, 0.91f, 1.0f);

        colors[ImGuiCol_Text] = ImVec4(0.09f, 0.15f, 0.24f, 1.0f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.40f, 0.38f, 1.0f);

        colors[ImGuiCol_CheckMark] = ImVec4(0.12f, 0.49f, 0.47f, 1.0f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.09f, 0.20f, 0.29f, 1.0f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.12f, 0.49f, 0.47f, 1.0f);

        colors[ImGuiCol_PlotLines] = ImVec4(0.12f, 0.49f, 0.47f, 1.0f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.09f, 0.20f, 0.29f, 1.0f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.09f, 0.20f, 0.29f, 1.0f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.85f, 0.52f, 0.20f, 1.0f);

        colors[ImGuiCol_DockingPreview] = ImVec4(0.12f, 0.49f, 0.47f, 0.35f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.93f, 0.90f, 0.86f, 1.0f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.92f, 0.89f, 0.85f, 1.0f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.81f, 0.76f, 0.71f, 1.0f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.88f, 0.84f, 0.79f, 1.0f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.98f, 0.97f, 0.95f, 0.75f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.12f, 0.49f, 0.47f, 1.0f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.08f, 0.11f, 0.15f, 0.28f);
    }

    ApplySharedStyle(style);
}

UIStyleHelper::ThemeMode UIStyleHelper::GetThemeMode() {
    return g_ThemeMode;
}

bool UIStyleHelper::IsDarkTheme() {
    return g_ThemeMode == ThemeMode::Dark;
}

ImU32 UIStyleHelper::GetSurfaceColor(int level) {
    if (IsDarkTheme()) {
        switch (level) {
            case 0: return IM_COL32(23, 31, 42, 255);
            case 1: return IM_COL32(31, 41, 54, 255);
            case 2: return IM_COL32(45, 59, 76, 255);
            default: return IM_COL32(63, 80, 101, 255);
        }
    }

    switch (level) {
        case 0: return Colors::Slate50;
        case 1: return Colors::Slate100;
        case 2: return Colors::Slate200;
        default: return Colors::Slate300;
    }
}

ImU32 UIStyleHelper::GetBorderColor() {
    return IsDarkTheme() ? IM_COL32(78, 94, 116, 255) : Colors::Slate200;
}

ImU32 UIStyleHelper::GetTextColor() {
    return IsDarkTheme() ? IM_COL32(232, 238, 244, 255) : Colors::Slate800;
}

ImU32 UIStyleHelper::GetMutedTextColor() {
    return IsDarkTheme() ? IM_COL32(153, 169, 186, 255) : Colors::Slate500;
}

ImU32 UIStyleHelper::GetSecondaryButtonColor() {
    return IsDarkTheme() ? IM_COL32(31, 41, 54, 255) : Colors::Slate100;
}

ImU32 UIStyleHelper::GetSecondaryButtonHoverColor() {
    return IsDarkTheme() ? IM_COL32(45, 59, 76, 255) : Colors::Slate200;
}

ImU32 UIStyleHelper::GetSecondaryButtonActiveColor() {
    return IsDarkTheme() ? IM_COL32(63, 80, 101, 255) : Colors::Slate300;
}

ImU32 UIStyleHelper::GetStatusColor(const std::string& status) {
    if (status == "active" || status == "online" || status == "complete") return Colors::Teal500;
    if (status == "processing" || status == "running" || status == "initializing") return Colors::Blue500;
    if (status == "idle" || status == "pending") return IsDarkTheme() ? IM_COL32(107, 122, 141, 255) : Colors::Slate400;
    if (status == "offline" || status == "error" || status == "failed") return Colors::Red500;
    if (status == "warning") return Colors::Copper500;
    return IsDarkTheme() ? GetMutedTextColor() : Colors::Slate500;
}

void UIStyleHelper::BeginCard(const char* title, const char*) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GetSurfaceColor(0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, GetBorderColor());
    
    ImGui::BeginChild(ImGui::GetID(title), ImVec2(0, 0), true);
    
    if (title) {
        ImGui::PushStyleColor(ImGuiCol_Text, GetTextColor());
        ImGui::TextUnformatted(title);
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();
    }
}

void UIStyleHelper::EndCard() {
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

bool UIStyleHelper::ModernButton(const char* label, const ImVec2& size) {
    ImGui::PushStyleColor(ImGuiCol_Button, GetSurfaceColor(2));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GetSurfaceColor(3));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IsDarkTheme() ? Colors::Blue600 : Colors::Slate400);
    ImGui::PushStyleColor(ImGuiCol_Text, GetTextColor());
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    
    bool result = ImGui::Button(label, size);
    
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    return result;
}

bool UIStyleHelper::ModernButton(const char* label, const char* tooltip, const ImVec2& size) {
    const bool pressed = ModernButton(label, size);
    UITooltip::Hover(tooltip);
    return pressed;
}

bool UIStyleHelper::ModernButtonPrimary(const char* label, const ImVec2& size) {
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::ForgeBlue500);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::ForgeBlue600);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::ForgeBlue600);
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    
    bool result = ImGui::Button(label, size);
    
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    return result;
}

bool UIStyleHelper::ModernButtonPrimary(const char* label, const char* tooltip, const ImVec2& size) {
    const bool pressed = ModernButtonPrimary(label, size);
    UITooltip::Hover(tooltip);
    return pressed;
}

bool UIStyleHelper::ModernButtonSecondary(const char* label, const ImVec2& size) {
    ImGui::PushStyleColor(ImGuiCol_Button, GetSecondaryButtonColor());
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GetSecondaryButtonHoverColor());
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, GetSecondaryButtonActiveColor());
    ImGui::PushStyleColor(ImGuiCol_Text, GetTextColor());
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, GetBorderColor());
    
    bool result = ImGui::Button(label, size);
    
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(2);
    return result;
}

bool UIStyleHelper::ModernButtonSecondary(const char* label, const char* tooltip, const ImVec2& size) {
    const bool pressed = ModernButtonSecondary(label, size);
    UITooltip::Hover(tooltip);
    return pressed;
}

void UIStyleHelper::ProgressBar(float fraction, const ImVec2& size, const char* overlay) {
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::ForgeBlue500);
    ImGui::ProgressBar(fraction, size, overlay);
    ImGui::PopStyleColor();
}

void UIStyleHelper::StatusBadge(const std::string& status) {
    ImU32 color = GetStatusColor(status);
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::SmallButton(status.c_str());
    ImGui::PopStyleColor(2);
}

void UIStyleHelper::Spacing(int count) {
    for (int i = 0; i < count; ++i) {
        ImGui::Spacing();
    }
}

void UIStyleHelper::HorizontalSeparator() {
    ImGui::PushStyleColor(ImGuiCol_Separator, GetBorderColor());
    ImGui::Separator();
    ImGui::PopStyleColor();
}

void UIStyleHelper::ApplyModernTheme() {
    ApplyTheme(ThemeMode::Light);
}

} // namespace SZM
