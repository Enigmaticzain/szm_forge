#include "UIThemeManager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

namespace SZM {

UIThemeManager* UIThemeManager::s_Instance = nullptr;

UIThemeManager& UIThemeManager::GetInstance() {
    if (s_Instance != nullptr) {
        return *s_Instance;
    }
    static UIThemeManager instance;
    s_Instance = &instance;
    return instance;
}

void UIThemeManager::ApplyTheme(UITheme theme) {
    m_CurrentTheme = theme;
    
    switch (theme) {
        case UITheme::Dark:
            ApplyDarkTheme();
            break;
        case UITheme::Light:
            ApplyLightTheme();
            break;
        case UITheme::HighContrast:
            ApplyHighContrastTheme();
            break;
        case UITheme::Engineering:
            ApplyEngineeringTheme();
            break;
    }
}

void UIThemeManager::ApplyDarkTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    
    m_AccentColor = IM_COL32(100, 150, 255, 255);
    m_WarningColor = IM_COL32(255, 200, 0, 255);
    m_ErrorColor = IM_COL32(255, 100, 100, 255);
    m_SuccessColor = IM_COL32(100, 255, 150, 255);
}

void UIThemeManager::ApplyLightTheme() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    
    m_AccentColor = IM_COL32(50, 100, 200, 255);
    m_WarningColor = IM_COL32(200, 150, 0, 255);
    m_ErrorColor = IM_COL32(200, 50, 50, 255);
    m_SuccessColor = IM_COL32(50, 200, 100, 255);
}

void UIThemeManager::ApplyHighContrastTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    
    auto& colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    m_AccentColor = IM_COL32(255, 255, 0, 255);
    m_WarningColor = IM_COL32(255, 128, 0, 255);
    m_ErrorColor = IM_COL32(255, 0, 0, 255);
    m_SuccessColor = IM_COL32(0, 255, 0, 255);
}

void UIThemeManager::ApplyEngineeringTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    
    auto& colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.35f, 0.50f, 1.0f);
    
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    
    m_AccentColor = IM_COL32(139, 92, 246, 255);
    m_WarningColor = IM_COL32(245, 158, 11, 255);
    m_ErrorColor = IM_COL32(239, 68, 68, 255);
    m_SuccessColor = IM_COL32(16, 185, 129, 255);
}

const char* UIThemeManager::GetThemeName(UITheme theme) const {
    switch (theme) {
        case UITheme::Dark:           return "Dark";
        case UITheme::Light:          return "Light";
        case UITheme::HighContrast:   return "High Contrast";
        case UITheme::Engineering:    return "Engineering";
        default:                      return "Unknown";
    }
}

void UIThemeManager::SaveThemePreference(const std::string& filepath) {
    using json = nlohmann::json;
    json data;
    data["theme"] = static_cast<int>(m_CurrentTheme);
    
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << data.dump(2);
        file.close();
    }
}

void UIThemeManager::LoadThemePreference(const std::string& filepath) {
    using json = nlohmann::json;
    std::ifstream file(filepath);
    
    if (!file.is_open()) return;
    
    try {
        json data;
        file >> data;
        if (data.contains("theme")) {
            ApplyTheme(static_cast<UITheme>(data["theme"].get<int>()));
        }
    } catch (const std::exception&) {
        // JSON parse error
    }
}

} // namespace SZM
