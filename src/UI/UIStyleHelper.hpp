#pragma once

#include <imgui.h>
#include <string>

namespace SZM {

/**
 * @class UIStyleHelper
 * @brief Modern UI styling utilities matching reference design
 */
class UIStyleHelper {
public:
    enum class ThemeMode {
        Light,
        Dark
    };

    // Color palette for the engineering desktop shell
    struct Colors {
        static constexpr ImU32 Slate50 = IM_COL32(248, 244, 238, 255);
        static constexpr ImU32 Slate100 = IM_COL32(240, 232, 221, 255);
        static constexpr ImU32 Slate200 = IM_COL32(225, 214, 201, 255);
        static constexpr ImU32 Slate300 = IM_COL32(202, 189, 173, 255);
        static constexpr ImU32 Slate400 = IM_COL32(145, 134, 121, 255);
        static constexpr ImU32 Slate500 = IM_COL32(108, 98, 88, 255);
        static constexpr ImU32 Slate600 = IM_COL32(85, 77, 69, 255);
        static constexpr ImU32 Slate700 = IM_COL32(50, 63, 82, 255);
        static constexpr ImU32 Slate800 = IM_COL32(23, 39, 61, 255);
        static constexpr ImU32 Slate900 = IM_COL32(14, 27, 43, 255);

        // Primary shell accents
        static constexpr ImU32 ForgeBlue500 = IM_COL32(23, 50, 75, 255);
        static constexpr ImU32 ForgeBlue600 = IM_COL32(16, 34, 57, 255);
        static constexpr ImU32 Teal500 = IM_COL32(31, 125, 120, 255);
        static constexpr ImU32 Teal600 = IM_COL32(20, 92, 89, 255);
        static constexpr ImU32 Copper500 = IM_COL32(216, 132, 50, 255);
        static constexpr ImU32 Copper600 = IM_COL32(187, 108, 31, 255);
        static constexpr ImU32 Violet500 = ForgeBlue500;
        static constexpr ImU32 Violet600 = ForgeBlue600;

        // Emerald palette (success)
        static constexpr ImU32 Emerald500 = IM_COL32(16, 185, 129, 255);
        static constexpr ImU32 Emerald600 = IM_COL32(5, 150, 105, 255);

        // Amber palette (warning)
        static constexpr ImU32 Amber500 = IM_COL32(245, 158, 11, 255);
        static constexpr ImU32 Amber600 = IM_COL32(217, 119, 6, 255);

        // Red palette (error)
        static constexpr ImU32 Red500 = IM_COL32(239, 68, 68, 255);
        static constexpr ImU32 Red600 = IM_COL32(220, 38, 38, 255);

        // Blue palette (info)
        static constexpr ImU32 Blue500 = IM_COL32(95, 136, 177, 255);
        static constexpr ImU32 Blue600 = IM_COL32(63, 105, 148, 255);
    };

    // Theme state
    static void ApplyTheme(ThemeMode theme);
    static ThemeMode GetThemeMode();
    static bool IsDarkTheme();

    // Theme-aware semantic colors
    static ImU32 GetSurfaceColor(int level = 0);
    static ImU32 GetBorderColor();
    static ImU32 GetTextColor();
    static ImU32 GetMutedTextColor();
    static ImU32 GetSecondaryButtonColor();
    static ImU32 GetSecondaryButtonHoverColor();
    static ImU32 GetSecondaryButtonActiveColor();

    // Status indicator colors
    static ImU32 GetStatusColor(const std::string& status);
    
    // Card styling
    static void BeginCard(const char* title, const char* icon = nullptr);
    static void EndCard();

    // Button styling
    static bool ModernButton(const char* label, const ImVec2& size = ImVec2(0, 0));
    static bool ModernButton(const char* label, const char* tooltip, const ImVec2& size = ImVec2(0, 0));
    static bool ModernButtonPrimary(const char* label, const ImVec2& size = ImVec2(0, 0));
    static bool ModernButtonPrimary(const char* label, const char* tooltip, const ImVec2& size = ImVec2(0, 0));
    static bool ModernButtonSecondary(const char* label, const ImVec2& size = ImVec2(0, 0));
    static bool ModernButtonSecondary(const char* label, const char* tooltip, const ImVec2& size = ImVec2(0, 0));

    // Progress bar
    static void ProgressBar(float fraction, const ImVec2& size, const char* overlay = nullptr);

    // Status badge
    static void StatusBadge(const std::string& status);

    // Spacing helpers
    static void Spacing(int count = 1);
    static void HorizontalSeparator();

    // Apply modern theme
    static void ApplyModernTheme();
};

} // namespace SZM
