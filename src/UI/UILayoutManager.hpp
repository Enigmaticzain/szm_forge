#pragma once

#include <string>
#include <vector>
#include <imgui.h>

namespace SZM {

/**
 * @enum LayoutPreset
 * @brief Pre-configured layout templates for different workflows
 */
enum class LayoutPreset {
    Default,        // Balanced layout for general use
    Modeling,       // Optimized for CAD/modeling work
    Simulation,     // Optimized for simulation and analysis
    Assembly,       // Optimized for assembly and kinematics
    Debug,          // Maximized console and debug panels
    Fullscreen      // Single viewport, minimal UI
};

/**
 * @class UILayoutManager
 * @brief Manages docking layouts and workspace organization
 * 
 * Responsibilities:
 *  - Apply preset layouts
 *  - Save/load custom layouts
 *  - Manage docking node configuration
 *  - Provide layout switching
 */
class UILayoutManager {
public:
    static UILayoutManager& GetInstance();

    // Layout application
    void ApplyLayout(LayoutPreset preset, ImGuiID dockspaceId);
    void ApplyCustomLayout(const std::string& filepath, ImGuiID dockspaceId);

    // Layout persistence
    void SaveCurrentLayout(const std::string& filepath);
    void LoadLayout(const std::string& filepath, ImGuiID dockspaceId);

    // Layout info
    const char* GetLayoutName(LayoutPreset preset) const;
    const char* GetLayoutDescription(LayoutPreset preset) const;

private:
    UILayoutManager() = default;
    ~UILayoutManager() = default;

    // Layout builders
    void BuildDefaultLayout(ImGuiID dockspaceId);
    void BuildModelingLayout(ImGuiID dockspaceId);
    void BuildSimulationLayout(ImGuiID dockspaceId);
    void BuildAssemblyLayout(ImGuiID dockspaceId);
    void BuildDebugLayout(ImGuiID dockspaceId);
    void BuildFullscreenLayout(ImGuiID dockspaceId);

    // Helper
    void DockPanelToNode(const char* panelName, ImGuiID nodeId);

    static UILayoutManager* s_Instance;
};

} // namespace SZM
