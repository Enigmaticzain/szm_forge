#pragma once

#include <imgui.h>
#include <string>

namespace SZM {

/**
 * @enum UITheme
 * @brief Available UI themes
 */
enum class UITheme {
    Dark,           // Professional dark theme
    Light,          // Professional light theme
    HighContrast,   // High contrast for accessibility
    Engineering     // Engineering-focused theme with accent colors
};

/**
 * @class UIThemeManager
 * @brief Manages UI styling and theme application
 * 
 * Responsibilities:
 *  - Apply theme presets
 *  - Manage color schemes
 *  - Provide consistent styling
 *  - Handle theme persistence
 */
class UIThemeManager {
public:
    static UIThemeManager& GetInstance();

    // Theme application
    void ApplyTheme(UITheme theme);
    void ApplyCustomTheme(const std::string& filepath);

    // Theme info
    const char* GetThemeName(UITheme theme) const;
    UITheme GetCurrentTheme() const { return m_CurrentTheme; }

    // Color utilities
    ImU32 GetAccentColor() const { return m_AccentColor; }
    ImU32 GetWarningColor() const { return m_WarningColor; }
    ImU32 GetErrorColor() const { return m_ErrorColor; }
    ImU32 GetSuccessColor() const { return m_SuccessColor; }

    // Persistence
    void SaveThemePreference(const std::string& filepath);
    void LoadThemePreference(const std::string& filepath);

private:
    UIThemeManager() = default;
    ~UIThemeManager() = default;

    // Theme builders
    void ApplyDarkTheme();
    void ApplyLightTheme();
    void ApplyHighContrastTheme();
    void ApplyEngineeringTheme();

    // Helper
    void SetupColors();

    static UIThemeManager* s_Instance;
    UITheme m_CurrentTheme = UITheme::Dark;

    // Theme colors
    ImU32 m_AccentColor = IM_COL32(100, 150, 255, 255);      // Blue
    ImU32 m_WarningColor = IM_COL32(255, 200, 0, 255);       // Yellow
    ImU32 m_ErrorColor = IM_COL32(255, 100, 100, 255);       // Red
    ImU32 m_SuccessColor = IM_COL32(100, 255, 150, 255);     // Green
};

} // namespace SZM
