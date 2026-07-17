namespace SZM {

    void Application::Init(const WindowConfig& config) {
        if (!glfwInit()) {
            throw std::runtime_error("SZM_FATAL: Failed to initialize GLFW.");
        }

        // Hint for future Vulkan/Custom Context (No default OpenGL context)
        // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); 

        // Handle DPI awareness across monitors
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

        m_WindowHandle = glfwCreateWindow(config.Width, config.Height, config.Title.c_str(), nullptr, nullptr);
        
        if (!m_WindowHandle) {
            glfwTerminate();
            throw std::runtime_error("SZM_FATAL: OS denied window creation.");
        }

        glfwMakeContextCurrent(m_WindowHandle);
        glfwSwapInterval(config.VSync ? 1 : 0); // V-Sync configuration

        glfwSetWindowUserPointer(m_WindowHandle, this);
        glfwSetFramebufferSizeCallback(m_WindowHandle, FramebufferResizeCallback);

        m_IsRunning = true;
        m_LastFrameTime = std::chrono::high_resolution_clock::now();
        
        std::cout << "SZM Forge: OS Window Initialized Successfully.\n";
    }

    void Application::Run() {
        int frameCount = 0;
        double timeAccumulator = 0.0;

        while (!glfwWindowShouldClose(m_WindowHandle) && m_IsRunning) {
            // 1. Calculate Delta Time (Tick)
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedTime = currentTime - m_LastFrameTime;
            m_DeltaTime = elapsedTime.count();
            m_LastFrameTime = currentTime;

            // 2. Process OS Events
            glfwPollEvents();

            // 3. Update Engines (Physics 200-Series will hook here via DeltaTime)
            // UpdateEngines(m_DeltaTime);

            // 4. Render (Placeholder swap buffers for testing V-Sync & stability)
            glClear(GL_COLOR_BUFFER_BIT); // Requires OpenGL context for testing phase
            glfwSwapBuffers(m_WindowHandle);

            // Diagnostic Framerate Output (per second)
            frameCount++;
            timeAccumulator += m_DeltaTime;
            if (timeAccumulator >= 1.0) {
                std::cout << "SZM Core Tick | FPS: " << frameCount << " | dt: " << m_DeltaTime << "s\n";
                frameCount = 0;
                timeAccumulator = 0.0;
            }
        }
    }

    void Application::Shutdown() {
        if (m_WindowHandle) {
            glfwDestroyWindow(m_WindowHandle);
            m_WindowHandle = nullptr;
        }
        glfwTerminate();
        std::cout << "SZM Forge: Terminated safely.\n";
    }

    void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        // Pauses rendering updates cleanly during rapid resize events (Edge case handling)
        if (width == 0 || height == 0) return; 
        glViewport(0, 0, width, height); 
    }

} // namespace SZM


// ===== NEW ADDITIONS FROM CODE BUNDLES =====

namespace SZM::Physics {

// Core Math Structures (Expandable to SIMD later)

// ===== NEW ADDITIONS FROM CODE BUNDLES =====

#include "RigidBody.hpp"
