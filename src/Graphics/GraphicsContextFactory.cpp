#include "GraphicsContext.hpp"
#include "OpenGLContext.hpp"
#include "VulkanContext.hpp"
#include <stdexcept>

namespace SZM::Graphics {

    std::unique_ptr<GraphicsContext>
    ContextFactory::Create(GLFWwindow* window, const ContextConfig& config) {
        switch (config.PreferredAPI) {
            case API::OpenGL:
                return std::make_unique<OpenGLContext>(window, config);
            case API::Vulkan:
                return std::make_unique<VulkanContext>(window, config);
            case API::DirectX12:
                throw std::runtime_error(
                    "[SZM Graphics] DX12 is not yet implemented for the "
                    "cross-platform target.");
            case API::None:
            default:
                throw std::runtime_error(
                    "[SZM FATAL] No Graphics API selected in ContextConfig.");
        }
    }

} // namespace SZM::Graphics
