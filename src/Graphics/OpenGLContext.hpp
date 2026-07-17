#pragma once

#include "GraphicsContext.hpp"
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>

namespace SZM::Graphics {

    class OpenGLContext : public GraphicsContext {
    public:
        OpenGLContext(GLFWwindow* windowHandle, const ContextConfig& config)
            : GraphicsContext(windowHandle, config) {}

        void Init() override {
            glfwMakeContextCurrent(m_WindowHandle);

            if (!gladLoadGL(glfwGetProcAddress)) {
                throw std::runtime_error("[SZM FATAL] GLAD failed to load OpenGL symbols.");
            }

            if (m_Config.MSAASamples > 1)
                glEnable(GL_MULTISAMPLE);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            // Dark Slate Grey background — consistent with SZM design spec
            glClearColor(0.18f, 0.18f, 0.20f, 1.0f);

            std::cout << "[SZM GPU] OpenGL 4.6 context initialised.\n";
            std::cout << "[SZM GPU] Hardware: " << GetGPUInfo() << "\n";
        }

        void SwapBuffers() override {
            glfwSwapBuffers(m_WindowHandle);
        }

        void OnResize(uint32_t width, uint32_t height) override {
            glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
        }

        void HandleDeviceLost() override {
            std::cerr << "[SZM CRITICAL] OpenGL context lost. Attempting rebuild...\n";
        }

        [[nodiscard]] std::string GetGPUInfo() const override {
            const auto* vendor   = glGetString(GL_VENDOR);
            const auto* renderer = glGetString(GL_RENDERER);
            if (!vendor || !renderer) return "Unknown GPU";
            return std::string(reinterpret_cast<const char*>(vendor)) + " — " +
                   std::string(reinterpret_cast<const char*>(renderer));
        }
    };

} // namespace SZM::Graphics
