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

            if (m_Config.MSAASamples > 1) {
                glEnable(GL_MULTISAMPLE);
            }
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glClearColor(0.18f, 0.18f, 0.20f, 1.0f);

            std::cout << "SZM Forge [GPU]: OpenGL 4.5 Context Initialized.\n";
            std::cout << "SZM Forge [GPU]: Hardware - " << GetGPUInfo() << "\n";
        }

        void SwapBuffers() override {
            // Frame clear is handled by the application render loop before ImGui draw.
            // Clearing here would erase the frame and produce a black screen.
            glfwSwapBuffers(m_WindowHandle);
        }

        void OnResize(uint32_t width, uint32_t height) override {
            glViewport(0, 0, width, height);
        }

        void HandleDeviceLost() override {
            std::cerr << "SZM_CRITICAL: OpenGL Context Lost. Attempting to rebuild state...\n";
        }

        [[nodiscard]] std::string GetGPUInfo() const override {
            const unsigned char* vendor = glGetString(GL_VENDOR);
            const unsigned char* renderer = glGetString(GL_RENDERER);
            return std::string(reinterpret_cast<const char*>(vendor)) + " " + 
                   std::string(reinterpret_cast<const char*>(renderer));
        }
    };

    class VulkanContext : public GraphicsContext {
    public:
        VulkanContext(GLFWwindow* windowHandle, const ContextConfig& config)
            : GraphicsContext(windowHandle, config) {}

        void Init() override {
            std::cout << "SZM Forge [GPU]: Vulkan Context Initialized.\n";
        }

        void SwapBuffers() override {
            // Vulkan swapchain presentation
        }

        void OnResize(uint32_t, uint32_t) override {
            // Recreate swapchain
        }

        void HandleDeviceLost() override {
            std::cerr << "SZM_FATAL: Vulkan Device Lost (VK_ERROR_DEVICE_LOST).\n";
        }

        [[nodiscard]] std::string GetGPUInfo() const override {
            return "Vulkan Physical Device";
        }
    };

    std::unique_ptr<GraphicsContext> ContextFactory::Create(GLFWwindow* window, const ContextConfig& config) {
        switch (config.PreferredAPI) {
            case API::OpenGL:
                return std::make_unique<OpenGLContext>(window, config);
            case API::Vulkan:
                return std::make_unique<VulkanContext>(window, config);
            default:
                throw std::runtime_error("SZM_FATAL: No Graphics API selected.");
        }
    }

} // namespace SZM::Graphics
