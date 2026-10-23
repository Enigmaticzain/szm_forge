# Phase 2 Activation Checklist — Enable ImGui

## Prerequisites

- [ ] ImGui docking branch cloned: `git clone -b docking https://github.com/ocornut/imgui.git external/imgui`
- [ ] GLFW development headers installed
- [ ] OpenGL 4.6 support available

## Step 1: Uncomment ImGui in CMakeLists.txt

**File:** `src/UI/CMakeLists.txt`

```cmake
# BEFORE:
add_library(SZM_UI_Module STATIC
    UIManager.cpp
    panels/SceneOutliner.cpp
    panels/ConsolePanel.cpp
    panels/PropertiesPanel.cpp
    panels/SimulationPanel.cpp
    panels/SimulationVisualizerPanel.cpp
    panels/ComponentBuilderPanel.cpp
    # ${IMGUI_DIR}/imgui.cpp
    # ...
)

# AFTER:
add_library(SZM_UI_Module STATIC
    UIManager.cpp
    panels/SceneOutliner.cpp
    panels/ConsolePanel.cpp
    panels/PropertiesPanel.cpp
    panels/SimulationPanel.cpp
    panels/SimulationVisualizerPanel.cpp
    panels/ComponentBuilderPanel.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# BEFORE:
target_include_directories(SZM_UI_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/panels
    # ${IMGUI_DIR}
    # ${IMGUI_DIR}/backends
)

# AFTER:
target_include_directories(SZM_UI_Module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/panels
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
```

## Step 2: Uncomment ImGui in UIManager.cpp

**File:** `src/UI/UIManager.cpp`

Uncomment all `ImGui::` calls and ImGui initialization code:

```cpp
// BEFORE:
void UIManager::Initialize() {
    // ImGuiIO& io = ImGui::GetIO();
    // ...
}

// AFTER:
void UIManager::Initialize() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}
```

And uncomment the `RenderUI()` implementation:

```cpp
// BEFORE:
void UIManager::RenderUI() {
    // ImGuiViewport* viewport = ImGui::GetMainViewport();
    // ...
    for (auto& panel : m_Panels) {
        if (panel && panel->IsVisible) {
            panel->Draw();
        }
    }
    // ImGui::End();
}

// AFTER:
void UIManager::RenderUI() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags host_flags =
        ImGuiWindowFlags_MenuBar        |
        ImGuiWindowFlags_NoDocking      |
        ImGuiWindowFlags_NoTitleBar     |
        ImGuiWindowFlags_NoCollapse     |
        ImGuiWindowFlags_NoResize       |
        ImGuiWindowFlags_NoMove         |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0.0f, 0.0f));
    ImGui::Begin("SZM_Forge_DockSpace", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("SZM_MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    if (m_FirstTimeDocking) {
        m_FirstTimeDocking = false;
        BuildDefaultDockspace(dockspace_id);
    }

    for (auto& panel : m_Panels) {
        if (panel && panel->IsVisible) {
            panel->Draw();
        }
    }

    ImGui::End();
}
```

## Step 3: Uncomment ImGui in Panel Draw() Methods

**Files:**
- `src/UI/panels/SimulationPanel.cpp`
- `src/UI/panels/SimulationVisualizerPanel.cpp`
- `src/UI/panels/ComponentBuilderPanel.cpp`

For each panel, uncomment the ImGui code in the `Draw()` method:

```cpp
// BEFORE:
void SimulationPanel::Draw() {
    // Uncomment when ImGui is linked:
    /*
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }
    // ... rest of code ...
    ImGui::End();
    */
}

// AFTER:
void SimulationPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }
    // ... rest of code ...
    ImGui::End();
}
```

## Step 4: Add ImGui Initialization to Application

**File:** `src/Core/Application.cpp`

Add ImGui context creation and backend setup:

```cpp
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void Application::Init(const WindowConfig& config) {
    // ... existing GLFW setup ...

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_WindowHandle, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // ... rest of initialization ...
}
```

## Step 5: Add ImGui Rendering to Main Loop

**File:** `src/Core/Application.cpp`

Update the `Run()` method:

```cpp
void Application::Run() {
    while (!glfwWindowShouldClose(m_WindowHandle) && m_IsRunning) {
        // ... existing code ...

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render UI
        UIManager::GetInstance().RenderUI();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Handle multi-viewport
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        // Swap buffers
        glfwSwapBuffers(m_WindowHandle);
    }
}
```

## Step 6: Add ImGui Cleanup to Shutdown

**File:** `src/Core/Application.cpp`

```cpp
void Application::Shutdown() {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // ... existing cleanup ...
}
```

## Step 7: Register Panels in Application

**File:** `src/main.cpp` or `src/Core/Application.cpp`

```cpp
#include "UI/UIManager.hpp"
#include "UI/panels/SimulationPanel.hpp"
#include "UI/panels/SimulationVisualizerPanel.hpp"
#include "UI/panels/ComponentBuilderPanel.hpp"

// In Application::Init() or main():
UIManager::GetInstance().Initialize();
UIManager::GetInstance().RegisterPanel(std::make_shared<SimulationPanel>());
UIManager::GetInstance().RegisterPanel(std::make_shared<SimulationVisualizerPanel>());
UIManager::GetInstance().RegisterPanel(std::make_shared<ComponentBuilderPanel>());
```

## Step 8: Build and Test

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run
./build/SZM_Forge
```

## Expected Result

When you run the application, you should see:

1. **Main window** with ImGui dockspace
2. **Three panels** in the dockspace:
   - "Simulation Control" — component list + controls
   - "Simulation Visualizer" — 3D viewport (placeholder)
   - "Component Builder" — material presets + quick-add

3. **Functionality:**
   - Add components via "Component Builder"
   - Select components in "Simulation Control"
   - Adjust force/heat sliders
   - Watch stress/temperature update in real-time
   - See color change from green (safe) to red (dangerous)

## Troubleshooting

### ImGui not rendering
- Check that `ImGui::NewFrame()` is called before `UIManager::RenderUI()`
- Verify `ImGui::Render()` is called after UI rendering
- Check OpenGL context is current

### Panels not appearing
- Verify `UIManager::RegisterPanel()` is called for each panel
- Check `panel->IsVisible` is `true`
- Verify `panel->Draw()` is being called

### Compilation errors
- Ensure ImGui headers are in include path
- Check `${IMGUI_DIR}` is set correctly in CMakeLists.txt
- Verify ImGui backend files are included in build

### Performance issues
- Reduce panel update frequency if needed
- Check for excessive EventBus subscriptions
- Profile with `ImGui::ShowMetricsWindow()`

## Next Steps After Activation

1. **Test all panels** — verify UI renders correctly
2. **Add camera controls** — implement orbit/pan/zoom
3. **Graphics integration** — render components into ImGui texture
4. **Material Database** — integrate with existing materials
5. **Assembly Graph** — build multi-component machines

---

**Estimated Time:** 30-60 minutes to complete all steps

**Difficulty:** Medium (mostly uncomment + add ImGui initialization)

**Status:** Ready to activate ⏳
