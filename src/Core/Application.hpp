#pragma once

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <chrono>
#include <memory>
#include "../UI/UIManager.hpp"
#include "Phase5PhysicsIntegration.hpp"

namespace SZM {
    namespace Graphics {
        class GraphicsContext;
    }

    struct WindowConfig {
        int         Width  = 1280;
        int         Height = 720;
        std::string Title  = "SZM Forge";
        bool        VSync  = true;
        bool        UseVulkan = false;
        std::string ThemePath = "";
    };

    /**
     * @class Application
     * @brief Singleton application host. Owns the OS window, the main loop,
     *        and the high-resolution frame timer. All sub-systems (UI, Input,
     *        Graphics) are initialised and torn down through this class.
     */
    class Application {
    public:
        // ----- Singleton ------------------------------------------------
        static Application& GetInstance() {
            static Application instance;
            return instance;
        }

        Application(const Application&)            = delete;
        Application& operator=(const Application&) = delete;

        // ----- Lifecycle ------------------------------------------------
        void Init(const WindowConfig& config);
        void Run();
        void Shutdown();

        // ----- Accessors ------------------------------------------------
        GLFWwindow* GetNativeWindow() const { return m_WindowHandle; }
        double      GetDeltaTime()    const { return m_DeltaTime;    }

    private:
        Application();
        ~Application();

        // GLFW Callbacks
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
        static void FocusCallback(GLFWwindow* window, int focused);

        // Internal helpers
        void TickSystems();
        bool LoadStartupLogoTexture();
        void ReleaseStartupLogoTexture();
        void RenderStartupSplash();

    private:
        GLFWwindow* m_WindowHandle = nullptr;
        bool        m_IsRunning    = false;
        bool        m_UseVulkan    = false;
        bool        m_ImGuiInitialized = false;
        std::unique_ptr<Graphics::GraphicsContext> m_GraphicsContext;

        double m_DeltaTime = 0.0;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFrameTime;
        UIManager m_UIManager;
        Phase5PhysicsIntegration m_Phase5PhysicsIntegration;
        bool m_ShowStartupSplash = true;
        double m_StartupSplashElapsed = 0.0;
        const double m_StartupSplashDuration = 5.5;
        GLuint m_StartupLogoTexture = 0;
        int m_StartupLogoWidth = 0;
        int m_StartupLogoHeight = 0;
        
        bool m_UseWebview = false;
        std::string m_WebviewUrl = "";
    };

} // namespace SZM
