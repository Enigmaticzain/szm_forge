#include "UILayoutManager.hpp"
#include <imgui_internal.h>
#include <nlohmann/json.hpp>
#include <fstream>

namespace SZM {

UILayoutManager* UILayoutManager::s_Instance = nullptr;

UILayoutManager& UILayoutManager::GetInstance() {
    if (s_Instance != nullptr) {
        return *s_Instance;
    }
    static UILayoutManager instance;
    s_Instance = &instance;
    return instance;
}

void UILayoutManager::ApplyLayout(LayoutPreset preset, ImGuiID dockspaceId) {
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

    switch (preset) {
        case LayoutPreset::Default:
            BuildDefaultLayout(dockspaceId);
            break;
        case LayoutPreset::Modeling:
            BuildModelingLayout(dockspaceId);
            break;
        case LayoutPreset::Simulation:
            BuildSimulationLayout(dockspaceId);
            break;
        case LayoutPreset::Assembly:
            BuildAssemblyLayout(dockspaceId);
            break;
        case LayoutPreset::Debug:
            BuildDebugLayout(dockspaceId);
            break;
        case LayoutPreset::Fullscreen:
            BuildFullscreenLayout(dockspaceId);
            break;
    }

    ImGui::DockBuilderFinish(dockspaceId);
}

void UILayoutManager::BuildDefaultLayout(ImGuiID dockspaceId) {
    ImGuiID dock_main = dockspaceId;
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.20f, nullptr, &dock_main);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.20f, nullptr, &dock_main);

    // Left: Scene hierarchy and library
    DockPanelToNode("Scene Outliner", dock_left);
    DockPanelToNode("Workshop Library", dock_left);

    // Right: Properties and inspector
    DockPanelToNode("Properties Inspector", dock_right);
    DockPanelToNode("Dashboard", dock_right);

    // Bottom: Console and diagnostics
    DockPanelToNode("SZM-CEA Console", dock_bottom);
    DockPanelToNode("Performance Monitor", dock_bottom);

    // Center: Main viewport
    DockPanelToNode("Workshop Viewport", dock_main);
    DockPanelToNode("Simulation Visualizer", dock_main);
}

void UILayoutManager::BuildModelingLayout(ImGuiID dockspaceId) {
    ImGuiID dock_main = dockspaceId;
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.15f, nullptr, &dock_main);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.30f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.15f, nullptr, &dock_main);

    // Left: Minimal - just outliner
    DockPanelToNode("Scene Outliner", dock_left);

    // Right: Properties and part editor
    DockPanelToNode("Properties Inspector", dock_right);
    DockPanelToNode("Part Editor", dock_right);

    // Bottom: Toolbar
    DockPanelToNode("Toolbar", dock_bottom);

    // Center: Large viewport
    DockPanelToNode("Workshop Viewport", dock_main);
}

void UILayoutManager::BuildSimulationLayout(ImGuiID dockspaceId) {
    ImGuiID dock_main = dockspaceId;
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.20f, nullptr, &dock_main);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.25f, nullptr, &dock_main);

    // Left: Scene and simulation control
    DockPanelToNode("Scene Outliner", dock_left);
    DockPanelToNode("Workshop Simulation", dock_left);

    // Right: Results and analysis
    DockPanelToNode("Analysis Results", dock_right);
    DockPanelToNode("Dashboard", dock_right);

    // Bottom: Playback and console
    DockPanelToNode("Simulation Playback", dock_bottom);
    DockPanelToNode("SZM-CEA Console", dock_bottom);

    // Center: Visualization
    DockPanelToNode("Simulation Visualizer", dock_main);
}

void UILayoutManager::BuildAssemblyLayout(ImGuiID dockspaceId) {
    ImGuiID dock_main = dockspaceId;
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.25f, nullptr, &dock_main);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.20f, nullptr, &dock_main);

    // Left: Assembly tree and library
    DockPanelToNode("Scene Outliner", dock_left);
    DockPanelToNode("Workshop Assembly", dock_left);
    DockPanelToNode("Workshop Library", dock_left);

    // Right: Properties and component builder
    DockPanelToNode("Properties Inspector", dock_right);
    DockPanelToNode("Component Builder", dock_right);

    // Bottom: Console
    DockPanelToNode("SZM-CEA Console", dock_bottom);

    // Center: Viewport
    DockPanelToNode("Workshop Viewport", dock_main);
}

void UILayoutManager::BuildDebugLayout(ImGuiID dockspaceId) {
    ImGuiID dock_main = dockspaceId;
    ImGuiID dock_top = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Up, 0.70f, nullptr, &dock_main);
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Left, 0.20f, nullptr, &dock_top);

    // Left: Scene
    DockPanelToNode("Scene Outliner", dock_left);

    // Top: Viewport
    DockPanelToNode("Workshop Viewport", dock_top);

    // Bottom: Console and debug panels
    DockPanelToNode("SZM-CEA Console", dock_main);
    DockPanelToNode("Performance Monitor", dock_main);
    DockPanelToNode("Visualization Debug", dock_main);
}

void UILayoutManager::BuildFullscreenLayout(ImGuiID dockspaceId) {
    // Just dock the viewport, hide everything else
    DockPanelToNode("Workshop Viewport", dockspaceId);
}

void UILayoutManager::DockPanelToNode(const char* panelName, ImGuiID nodeId) {
    ImGui::DockBuilderDockWindow(panelName, nodeId);
}

void UILayoutManager::SaveCurrentLayout(const std::string& filepath) {
    // ImGui handles layout saving internally via ini files
    // This is a placeholder for custom layout serialization
}

void UILayoutManager::LoadLayout(const std::string& filepath, ImGuiID dockspaceId) {
    // ImGui handles layout loading internally via ini files
    // This is a placeholder for custom layout deserialization
}

const char* UILayoutManager::GetLayoutName(LayoutPreset preset) const {
    switch (preset) {
        case LayoutPreset::Default:     return "Default";
        case LayoutPreset::Modeling:    return "Modeling";
        case LayoutPreset::Simulation:  return "Simulation";
        case LayoutPreset::Assembly:    return "Assembly";
        case LayoutPreset::Debug:       return "Debug";
        case LayoutPreset::Fullscreen:  return "Fullscreen";
        default:                        return "Unknown";
    }
}

const char* UILayoutManager::GetLayoutDescription(LayoutPreset preset) const {
    switch (preset) {
        case LayoutPreset::Default:
            return "Balanced layout for general use";
        case LayoutPreset::Modeling:
            return "Optimized for CAD and modeling work";
        case LayoutPreset::Simulation:
            return "Optimized for simulation and analysis";
        case LayoutPreset::Assembly:
            return "Optimized for assembly and kinematics";
        case LayoutPreset::Debug:
            return "Maximized console and debug output";
        case LayoutPreset::Fullscreen:
            return "Single viewport, minimal UI";
        default:
            return "Unknown layout";
    }
}

} // namespace SZM
