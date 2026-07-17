#pragma once

#include "../EasyTabBar.hpp"

namespace SZM {

/**
 * @class SimpleExamplePanel
 * @brief Beginner-friendly example showing how to use tabs
 * 
 * This is the simplest possible way to create a tabbed panel!
 */
class SimpleExamplePanel : public BeginnerPanel {
public:
    SimpleExamplePanel() : BeginnerPanel("Simple Example") {
        // Just add tabs - that's all!
        AddTab("home", "Home", "🏠", [this]() { DrawHome(); });
        AddTab("settings", "Settings", "⚙️", [this]() { DrawSettings(); });
        AddTab("about", "About", "ℹ️", [this]() { DrawAbout(); });
    }

private:
    // Tab 1: Home
    void DrawHome() {
        ImGui::TextUnformatted("Welcome to SZM Forge!");
        ImGui::Spacing();
        ImGui::TextUnformatted("This is a simple tabbed panel.");
        ImGui::TextUnformatted("Click the tabs above to switch between sections.");
    }

    // Tab 2: Settings
    void DrawSettings() {
        static bool enableFeature = true;
        static int quality = 2;

        ImGui::Checkbox("Enable Feature", &enableFeature);
        ImGui::SliderInt("Quality", &quality, 1, 5);
        
        ImGui::Spacing();
        if (ImGui::Button("Save Settings")) {
            // Save logic here
        }
    }

    // Tab 3: About
    void DrawAbout() {
        ImGui::TextUnformatted("SZM Forge v0.3.0");
        ImGui::TextUnformatted("Modern UI System");
        ImGui::Spacing();
        ImGui::TextUnformatted("Built with C++20 and ImGui");
    }
};

} // namespace SZM
