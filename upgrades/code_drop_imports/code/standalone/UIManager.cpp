#include "UIManager.h"
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

import os
import shutil

# --- 1. Define the File Paths and Contents ---
file_system = {
    "SZM_Forge_UI/CMakeLists.txt": """# SZM_Forge_UI/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(SZM_Forge_UI_Module)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)

add_library(SZM_UI_Module STATIC
    src/UIManager.cpp
    src/panels/SceneOutliner.cpp
    src/panels/ConsolePanel.cpp
    # Add ImGui source files here in your actual build environment
)

target_include_directories(SZM_UI_Module PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/panels
)

target_compile_features(SZM_UI_Module PUBLIC cxx_std_20)
""",

    "SZM_Forge_UI/src/EventBus.h": """// SZM_Forge_UI/src/EventBus.h
#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <any>

class EventBus {
public:
    using EventCallback = std::function<void(const std::any&)>;

    static EventBus& GetInstance() {
        static EventBus instance;
        return instance;
    }

    void Subscribe(const std::string& eventName, EventCallback callback) {
        listeners[eventName].push_back(callback);
    }

    void Publish(const std::string& eventName, const std::any& payload) {
        if (listeners.find(eventName) != listeners.end()) {
            for (auto& callback : listeners[eventName]) {
                callback(payload);
            }
        }
    }
private:
    std::unordered_map<std::string, std::vector<EventCallback>> listeners;
};
""",

    "SZM_Forge_UI/src/IPanel.h": """// SZM_Forge_UI/src/IPanel.h
#pragma once
#include <string>

class IPanel {
public:
    virtual ~IPanel() = default;
    virtual void Draw() = 0;
    virtual std::string GetTitle() const = 0;
    
    bool IsVisible = true;
};
""",

    "SZM_Forge_UI/src/UIManager.h": """// SZM_Forge_UI/src/UIManager.h
#pragma once
#include <vector>
#include <memory>
#include "IPanel.h"

class UIManager {
public:
    UIManager();
    ~UIManager();

    void Initialize();
    void RegisterPanel(std::shared_ptr<IPanel> panel);
    void RenderUI();
    void BuildDefaultDockspace(unsigned int dockspaceId);

private:
    std::vector<std::shared_ptr<IPanel>> m_Panels;
    bool m_FirstTimeDocking = true;
};
""",

    "SZM_Forge_UI/src/UIManager.cpp": """// SZM_Forge_UI/src/UIManager.cpp
#include "UIManager.h"
// #include <imgui.h> // Uncomment when ImGui is linked
// #include <imgui_internal.h>

UIManager::UIManager() {}
UIManager::~UIManager() {}

void UIManager::Initialize() {
    // ImGui Initialization logic goes here
}

void UIManager::RegisterPanel(std::shared_ptr<IPanel> panel) {
    m_Panels.push_back(panel);
}

void UIManager::RenderUI() {
    // Main docking logic and panel rendering loop goes here
}

void UIManager::BuildDefaultDockspace(unsigned int dockspace_id) {
    // Default docking splits logic goes here
}
""",

    "SZM_Forge_UI/src/panels/SceneOutliner.h": """// SZM_Forge_UI/src/panels/SceneOutliner.h
#pragma once
#include "IPanel.h"
#include <vector>
#include <string>

class SceneOutliner : public IPanel {
public:
    SceneOutliner();
    void Draw() override;
    std::string GetTitle() const override { return "Scene Outliner"; }

private:
    // Mock data for the tree nodes
    std::vector<std::string> m_RootNodes;
};
""",

    "SZM_Forge_UI/src/panels/SceneOutliner.cpp": """// SZM_Forge_UI/src/panels/SceneOutliner.cpp
#include "SceneOutliner.h"
// #include <imgui.h>

SceneOutliner::SceneOutliner() {
    m_RootNodes = {"Engine_Assembly_01", "Chassis_Frame", "Sensor_Array"};
}

void SceneOutliner::Draw() {
    // ImGui::Begin(GetTitle().c_str(), &IsVisible);
    // for (const auto& node : m_RootNodes) {
    //     if (ImGui::TreeNode(node.c_str())) {
    //         ImGui::Text("Child Node...");
    //         ImGui::TreePop();
    //     }
    // }
    // ImGui::End();
}
""",

    "SZM_Forge_UI/src/panels/ConsolePanel.h": """// SZM_Forge_UI/src/panels/ConsolePanel.h
#pragma once
#include "IPanel.h"
#include <vector>
#include <string>

class ConsolePanel : public IPanel {
public:
    ConsolePanel();
    void Draw() override;
    std::string GetTitle() const override { return "SZM-CEA Console"; }
    void AddLog(const std::string& log);

private:
    std::vector<std::string> m_Logs;
    char m_InputBuffer[256] = "";
};
""",

    "SZM_Forge_UI/src/panels/ConsolePanel.cpp": """// SZM_Forge_UI/src/panels/ConsolePanel.cpp
#include "ConsolePanel.h"
// #include <imgui.h>

ConsolePanel::ConsolePanel() {
    AddLog("[System] SZM-CEA AI Assistant Initialized.");
}

void ConsolePanel::AddLog(const std::string& log) {
    m_Logs.push_back(log);
}

void ConsolePanel::Draw() {
    // ImGui::Begin(GetTitle().c_str(), &IsVisible);
    // for (const auto& log : m_Logs) {
    //     ImGui::TextUnformatted(log.c_str());
    // }
    // ImGui::InputText("Command", m_InputBuffer, 256);
    // ImGui::End();
}
"""
}

# --- 2. Create Folders and Write Files ---
print("Creating directories and writing C++ files...")
for file_path, content in file_system.items():
    directory = os.path.dirname(file_path)
    if not os.path.exists(directory):
        os.makedirs(directory)
    
    with open(file_path, 'w') as f:
        f.write(content)
    print(f"Created: {file_path}")

# --- 3. Generate the ZIP Archive ---
print("\\nZipping the module...")
shutil.make_archive("SZM_Forge_UI_Module", 'zip', "SZM_Forge_UI")
print("Success! Created SZM_Forge_UI_Module.zip")

cmake_minimum_required(VERSION 3.20)
project(SZM_Forge_InputCore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific definitions
if(WIN32)
    add_compile_definitions(SZM_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(SZM_PLATFORM_LINUX)
endif()

# Define the InputCore Library
add_library(AppCore_Input_Event_Bus STATIC
    src/InputManager.cpp
    src/SpaceMouseDriver.cpp
)

target_include_directories(AppCore_Input_Event_Bus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Note: Link against your OS Windowing API (e.g., GLFW, SDL2, or native Win32/X11)
# target_link_libraries(AppCore_Input_Event_Bus PUBLIC glfw)

#pragma once
#include <functional>
#include <string>

namespace SZM {

    enum class EventType {
        None = 0,
        WindowLostFocus,
        KeyPressed, KeyReleased, KeyTyped,
        MouseMoved, MouseButtonPressed, MouseButtonReleased, MouseScrolled,
        SpaceMouseMoved, SpaceMouseButton
    };

    enum EventCategory {
        None = 0,
        EventCategoryInput      = 1 << 0,
        EventCategoryKeyboard   = 1 << 1,
        EventCategoryMouse      = 1 << 2,
        EventCategorySpaceMouse = 1 << 3
    };

    // Base Event Class
    class Event {
    public:
        virtual ~Event() = default;
        bool Handled = false; // The core routing mechanism (UI vs 3D Viewport)

        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); }

        bool IsInCategory(EventCategory category) {
            return GetCategoryFlags() & category;
        }
    };

    // Event Dispatcher for Routing
    class EventDispatcher {
    public:
        EventDispatcher(Event& event) : m_Event(event) {}

        // F will be deduced by the compiler
        template<typename T, typename F>
        bool Dispatch(const F& func) {
            if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled) {
                m_Event.Handled = func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };

} // namespace SZM

#pragma once
#include "Event.hpp"
#include <sstream>

namespace SZM {

    // --- KEYBOARD EVENTS ---
    class KeyPressedEvent : public Event {
    public:
        KeyPressedEvent(int keycode, int repeatCount) 
            : m_KeyCode(keycode), m_RepeatCount(repeatCount) {}

        int GetKeyCode() const { return m_KeyCode; }
        
        static EventType GetStaticType() { return EventType::KeyPressed; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyPressed"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryKeyboard; }

    private:
        int m_KeyCode;
        int m_RepeatCount;
    };

    // --- MOUSE EVENTS ---
    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

        float GetX() const { return m_MouseX; }
        float GetY() const { return m_MouseY; }

        static EventType GetStaticType() { return EventType::MouseMoved; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseMoved"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }

    private:
        float m_MouseX, m_MouseY;
    };

    // --- SPACEMOUSE EVENTS (6-DOF) ---
    class SpaceMouseMovedEvent : public Event {
    public:
        // Tx, Ty, Tz (Translation) | Rx, Ry, Rz (Rotation)
        SpaceMouseMovedEvent(float tx, float ty, float tz, float rx, float ry, float rz)
            : m_Tx(tx), m_Ty(ty), m_Tz(tz), m_Rx(rx), m_Ry(ry), m_Rz(rz) {}

        static EventType GetStaticType() { return EventType::SpaceMouseMoved; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "SpaceMouseMoved"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategorySpaceMouse; }

        // Data getters...
    private:
        float m_Tx, m_Ty, m_Tz, m_Rx, m_Ry, m_Rz;
    };

} // namespace SZM

#pragma once
#include <array>
#include <utility>

namespace SZM {

    class InputManager {
    public:
        static void Init();
        
        // Polling APIs for systems that prefer direct state checking over events
        static bool IsKeyPressed(int keycode);
        static bool IsMouseButtonPressed(int button);
        static std::pair<float, float> GetMousePosition();

        // State updaters (called by OS-level window callbacks)
        static void UpdateKeyState(int keycode, bool pressed);
        static void UpdateMouseState(int button, bool pressed);
        static void UpdateMousePosition(float x, float y);

        // Edge Case Handling: Called when App loses OS focus
        static void ClearStateOnFocusLost();

    private:
        // Internal state trackers (size depends on underlying API, e.g., GLFW_KEY_LAST is 348)
        static std::array<bool, 512> s_KeyStates;
        static std::array<bool, 8> s_MouseButtonStates;
        static float s_MouseX, s_MouseY;
    };

} // namespace SZM

#include "InputManager.hpp"
#include <algorithm>

namespace SZM {

    std::array<bool, 512> InputManager::s_KeyStates = {false};
    std::array<bool, 8> InputManager::s_MouseButtonStates = {false};
    float InputManager::s_MouseX = 0.0f;
    float InputManager::s_MouseY = 0.0f;

    void InputManager::Init() {
        ClearStateOnFocusLost();
    }

    bool InputManager::IsKeyPressed(int keycode) {
        if (keycode >= 0 && keycode < s_KeyStates.size())
            return s_KeyStates[keycode];
        return false;
    }

    void InputManager::UpdateKeyState(int keycode, bool pressed) {
        if (keycode >= 0 && keycode < s_KeyStates.size()) {
            s_KeyStates[keycode] = pressed;
        }
    }

    // Resolves Edge Case 11: Alt-Tabbing away causes keys to get "stuck"
    void InputManager::ClearStateOnFocusLost() {
        std::fill(s_KeyStates.begin(), s_KeyStates.end(), false);
        std::fill(s_MouseButtonStates.begin(), s_MouseButtonStates.end(), false);
    }

    // Additional implementations for Mouse state follow similarly...
}

// Inside your Application's Main Event Callback
void SZM_App::OnEvent(SZM::Event& e) {
    SZM::EventDispatcher dispatcher(e);

    // 1. Dispatch to UI Layer FIRST
    // If the UI is focused on a text box and the event is a 'W' keypress, 
    // m_UIManager.OnEvent will process it and return TRUE.
    dispatcher.Dispatch<SZM::KeyPressedEvent>(std::bind(&UIManager::OnEvent, &m_UIManager, std::placeholders::_1));

    // 2. Dispatch to 3D Camera/Viewport LAST
    // Because dispatcher checks `!m_Event.Handled`, if the UI consumed the event,
    // this dispatch will simply be skipped. The 3D camera will NOT move forward.
    dispatcher.Dispatch<SZM::KeyPressedEvent>(std::bind(&CameraController::OnEvent, &m_CameraController, std::placeholders::_1));
}

#pragma once

#include <memory>
#include <string>
#include <cstdint>

// Forward declaration of GLFWwindow to avoid including GLFW here
struct GLFWwindow;

namespace SZM::Graphics {

    enum class API {
        None = 0,
        OpenGL = 1,
        Vulkan = 2,
        DirectX12 = 3 // Reserved for future Windows-only optimizations
    };

    struct ContextConfig {
        API PreferredAPI = API::Vulkan;
        uint8_t MSAASamples = 4;        // 2x, 4x, 8x for CAD line smoothing
        uint8_t DepthBits = 24;         // Minimum 24-bit to prevent Z-fighting
        bool DoubleBuffering = true;
    };

    class GraphicsContext {
    public:
        virtual ~GraphicsContext() = default;

        // Core Lifecycle
        virtual void Init() = 0;
        virtual void SwapBuffers() = 0;
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
        
        // Edge Case Handling
        virtual void HandleDeviceLost() = 0;

        // Hardware Query
        [[nodiscard]] virtual std::string GetGPUInfo() const = 0;

    protected:
        GLFWwindow* m_WindowHandle;
        ContextConfig m_Config;

        GraphicsContext(GLFWwindow* windowHandle, const ContextConfig& config)
            : m_WindowHandle(windowHandle), m_Config(config) {}
    };

    // Factory Class for initializing the correct backend
    class ContextFactory {
    public:
        [[nodiscard]] static std::unique_ptr<GraphicsContext> Create(GLFWwindow* window, const ContextConfig& config);
    };

} // namespace SZM::Graphics

#include "GraphicsContext.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

namespace SZM::Graphics {

    class OpenGLContext : public GraphicsContext {
    public:
        OpenGLContext(GLFWwindow* windowHandle, const ContextConfig& config)
            : GraphicsContext(windowHandle, config) {}

        void Init() override {
            glfwMakeContextCurrent(m_WindowHandle);

            // In a full build, initialize GLAD/GLEW here to load OpenGL 4.6 function pointers.
            // if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { ... }

            // Configure MSAA and Depth Buffering based on input state
            if (m_Config.MSAASamples > 1) {
                glEnable(GL_MULTISAMPLE);
            }
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            // Dark Slate Grey [0.18, 0.18, 0.2] as per the test case
            glClearColor(0.18f, 0.18f, 0.20f, 1.0f);

            std::cout << "SZM Forge [GPU]: OpenGL 4.6 Context Initialized.\n";
            std::cout << "SZM Forge [GPU]: Hardware - " << GetGPUInfo() << "\n";
        }

        void SwapBuffers() override {
            // Clears both color and the 24-bit depth buffer for the next 3D frame
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glfwSwapBuffers(m_WindowHandle);
        }

        void OnResize(uint32_t width, uint32_t height) override {
            glViewport(0, 0, width, height);
            // OpenGL manages its own swapchain inherently, but we update the viewport.
        }

        void HandleDeviceLost() override {
            std::cerr << "SZM_CRITICAL: OpenGL Context Lost. Attempting to rebuild state...\n";
            // OpenGL context recovery logic (rare, but necessary for robust CAD)
        }

        [[nodiscard]] std::string GetGPUInfo() const override {
            const unsigned char* vendor = glGetString(GL_VENDOR);
            const unsigned char* renderer = glGetString(GL_RENDERER);
            return std::string(reinterpret_cast<const char*>(vendor)) + " " + 
                   std::string(reinterpret_cast<const char*>(renderer));
        }
    };

} // namespace SZM::Graphics

#include "GraphicsContext.hpp"
#include <iostream>

namespace SZM::Graphics {

    class VulkanContext : public GraphicsContext {
    public:
        VulkanContext(GLFWwindow* windowHandle, const ContextConfig& config)
            : GraphicsContext(windowHandle, config) {}

        void Init() override {
            CreateInstance();
            SelectPhysicalDevice(); // Query OS -> Discrete GPU preference
            CreateLogicalDevice();
            CreateSwapchain();      // Match OS Window dims & Double/Triple buffering
            CreateDepthResources(); // Allocate 24-bit depth + 8-bit stencil memory
            
            std::cout << "SZM Forge [GPU]: Vulkan Context Initialized.\n";
        }

        void SwapBuffers() override {
            // 1. vkAcquireNextImageKHR()
            // 2. vkWaitForFences()
            // 3. vkQueueSubmit() -> Submit Command Buffers from 100-Series
            // 4. vkQueuePresentKHR()
        }

        void OnResize(uint32_t width, uint32_t height) override {
            // Explicit Swapchain destruction and recreation required to prevent stretching/crashing
            RecreateSwapchain();
        }

        void HandleDeviceLost() override {
            // Handle VK_ERROR_DEVICE_LOST
            std::cerr << "SZM_FATAL: Vulkan Device Lost (VK_ERROR_DEVICE_LOST). Re-initializing physical device...\n";
        }

        [[nodiscard]] std::string GetGPUInfo() const override {
            return "Vulkan Physical Device Name (Pending Query)";
        }

    private:
        // Vulkan handles (vk::Instance, vk::Device, vk::SwapchainKHR, etc.) would live here.
        void CreateInstance() {}
        void SelectPhysicalDevice() {}
        void CreateLogicalDevice() {}
        void CreateSwapchain() {}
        void RecreateSwapchain() {}
        void CreateDepthResources() {}
    };

} // namespace SZM::Graphics

#include "GraphicsContext.hpp"
// In a real build, these would be separate headers
#include "OpenGLContext.cpp" 
#include "VulkanContext.cpp" 

namespace SZM::Graphics {

    std::unique_ptr<GraphicsContext> ContextFactory::Create(GLFWwindow* window, const ContextConfig& config) {
        switch (config.PreferredAPI) {
            case API::OpenGL:
                return std::make_unique<OpenGLContext>(window, config);
            case API::Vulkan:
                return std::make_unique<VulkanContext>(window, config);
            case API::DirectX12:
                throw std::runtime_error("DX12 not currently implemented for cross-platform target.");
            case API::None:
            default:
                throw std::runtime_error("SZM_FATAL: No Graphics API selected.");
        }
    }

} // namespace SZM::Graphics

#pragma once
#include <functional>
#include <string>

namespace SZM {
    enum class EventType {
        None = 0, WindowLostFocus,
        KeyPressed, KeyReleased, KeyTyped,
        MouseMoved, MouseButtonPressed, MouseButtonReleased, MouseScrolled,
        SpaceMouseMoved, SpaceMouseButton
    };

    enum EventCategory {
        None = 0,
        EventCategoryInput      = 1 << 0,
        EventCategoryKeyboard   = 1 << 1,
        EventCategoryMouse      = 1 << 2,
        EventCategorySpaceMouse = 1 << 3
    };

    class Event {
    public:
        virtual ~Event() = default;
        bool Handled = false; 
        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        bool IsInCategory(EventCategory category) { return GetCategoryFlags() & category; }
    };

    class EventDispatcher {
    public:
        EventDispatcher(Event& event) : m_Event(event) {}
        template<typename T, typename F>
        bool Dispatch(const F& func) {
            if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled) {
                m_Event.Handled = func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };
}

#pragma once
#include "Event.hpp"

namespace SZM {
    class KeyPressedEvent : public Event {
    public:
        KeyPressedEvent(int keycode, int repeatCount) : m_KeyCode(keycode), m_RepeatCount(repeatCount) {}
        static EventType GetStaticType() { return EventType::KeyPressed; }
        virtual EventType GetEventType() const override { return GetStaticType(); }
        virtual const char* GetName() const override { return "KeyPressed"; }
        virtual int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryKeyboard; }
        int GetKeyCode() const { return m_KeyCode; }
    private:
        int m_KeyCode; int m_RepeatCount;
    };

    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}
        static EventType GetStaticType() { return EventType::MouseMoved; }
        virtual EventType GetEventType() const override { return GetStaticType(); }
        virtual const char* GetName() const override { return "MouseMoved"; }
        virtual int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }
    private:
        float m_MouseX, m_MouseY;
    };
}

#pragma once
#include <array>

namespace SZM {
    class InputManager {
    public:
        static void UpdateKeyState(int keycode, bool pressed);
        static bool IsKeyPressed(int keycode);
        static void ClearState(); // For Alt-Tab edge case
    private:
        static std::array<bool, 512> s_KeyStates;
    };
}

#include "InputManager.hpp"
#include <algorithm>

namespace SZM {
    std::array<bool, 512> InputManager::s_KeyStates = {false};

    void InputManager::UpdateKeyState(int keycode, bool pressed) {
        if (keycode >= 0 && keycode < 512) s_KeyStates[keycode] = pressed;
    }

    bool InputManager::IsKeyPressed(int keycode) {
        return (keycode >= 0 && keycode < 512) ? s_KeyStates[keycode] : false;
    }

    void InputManager::ClearState() {
        std::fill(s_KeyStates.begin(), s_KeyStates.end(), false);
    }
}

add_library(AppCore_Input_Event_Bus STATIC
    ../../src/input/InputManager.cpp
)

target_include_directories(AppCore_Input_Event_Bus PUBLIC 
    ../../src/input
)

