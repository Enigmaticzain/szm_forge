#include <imgui_internal.h>

UIManager::UIManager() {}
UIManager::~UIManager() {}

void UIManager::Initialize() {
    // ImGui context setup is assumed to be handled in Module 010.1
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Monitor Tear-off

    // Professional Dark Theme setup
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

void UIManager::RegisterPanel(std::shared_ptr<IPanel> panel) {
    m_Panels.push_back(panel);
}

void UIManager::RenderUI() {
    // 1. Create the Central Dockspace spanning the OS window
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("SZM_Forge_DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    // 2. Initial Layout Construction (Only runs once if no config is found)
    if (m_FirstTimeDocking) {
        m_FirstTimeDocking = false;
        BuildDefaultDockspace(dockspace_id);
    }

    // 3. Render all registered panels
    for (auto& panel : m_Panels) {
        if (panel->IsVisible) {
            panel->Draw();
        }
    }

    ImGui::End(); // End SZM_Forge_DockSpace
}

void UIManager::BuildDefaultDockspace(uint32_t dockspace_id) {
    ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    // Split the dockspace
    ImGuiID dock_main_id = dockspace_id;
    ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
    ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
    ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

    // Route specific panels to specific docking nodes
    ImGui::DockBuilderDockWindow("Scene Outliner", dock_id_left);
    ImGui::DockBuilderDockWindow("Properties Inspector", dock_id_right);
    ImGui::DockBuilderDockWindow("SZM-CEA Console", dock_id_bottom);
    ImGui::DockBuilderDockWindow("3D Viewport", dock_main_id);

    ImGui::DockBuilderFinish(dockspace_id);
}

# CMakeLists.txt (Snippet for UI Module)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)

# Ensure you clone the "docking" branch of Dear ImGui
# git clone -b docking https://github.com/ocornut/imgui.git external/imgui

add_library(SZM_UI_Module STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/UIManager.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    # Add your specific backend files here (e.g., imgui_impl_glfw.cpp, imgui_impl_opengl3.cpp)
)

target_include_directories(SZM_UI_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
)

# C++20 standard enforcement
target_compile_features(SZM_UI_Module PUBLIC cxx_std_20)
