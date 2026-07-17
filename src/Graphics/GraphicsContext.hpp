#pragma once

#include <memory>
#include <string>
#include <cstdint>

struct GLFWwindow;

namespace SZM::Graphics {

    enum class API {
        None = 0,
        OpenGL = 1,
        Vulkan = 2,
        DirectX12 = 3
    };

    struct ContextConfig {
        API PreferredAPI = API::Vulkan;
        uint8_t MSAASamples = 4;
        uint8_t DepthBits = 24;
        bool DoubleBuffering = true;
    };

    class GraphicsContext {
    public:
        virtual ~GraphicsContext() = default;

        virtual void Init() = 0;
        virtual void SwapBuffers() = 0;
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
        virtual void HandleDeviceLost() = 0;

        [[nodiscard]] virtual std::string GetGPUInfo() const = 0;

    protected:
        GLFWwindow* m_WindowHandle;
        ContextConfig m_Config;

        GraphicsContext(GLFWwindow* windowHandle, const ContextConfig& config)
            : m_WindowHandle(windowHandle), m_Config(config) {}
    };

    class ContextFactory {
    public:
        [[nodiscard]] static std::unique_ptr<GraphicsContext> Create(GLFWwindow* window, const ContextConfig& config);
    };

} // namespace SZM::Graphics
