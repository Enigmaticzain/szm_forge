#pragma once

#include "GraphicsContext.hpp"
#include <iostream>

namespace SZM::Graphics {

    /**
     * @class VulkanContext
     * @brief Vulkan 1.3 rendering backend.
     *
     * Status: Scaffold / Stub — full implementation pending 100-Series engineers.
     * All critical entry-points are defined and documented so the rest of the
     * system compiles against a valid interface today.
     */
    class VulkanContext : public GraphicsContext {
    public:
        VulkanContext(GLFWwindow* windowHandle, const ContextConfig& config)
            : GraphicsContext(windowHandle, config) {}

        void Init() override {
            CreateInstance();
            SelectPhysicalDevice();
            CreateLogicalDevice();
            CreateSwapchain();
            CreateDepthResources();
            std::cout << "[SZM GPU] Vulkan context scaffold initialised.\n";
        }

        void SwapBuffers() override {
            // Full implementation:
            // 1. vkAcquireNextImageKHR()
            // 2. vkWaitForFences()
            // 3. vkQueueSubmit()    — command buffers from 100-Series
            // 4. vkQueuePresentKHR()
        }

        void OnResize(uint32_t /*width*/, uint32_t /*height*/) override {
            // Explicit swapchain destruction & recreation required
            RecreateSwapchain();
        }

        void HandleDeviceLost() override {
            std::cerr << "[SZM FATAL] Vulkan device lost (VK_ERROR_DEVICE_LOST). "
                         "Re-initialising physical device...\n";
        }

        [[nodiscard]] std::string GetGPUInfo() const override {
            return "Vulkan Physical Device (pending vkGetPhysicalDeviceProperties)";
        }

    private:
        // Vulkan object handles (VkInstance, VkDevice, VkSwapchainKHR …)
        // will be declared here once the 100-Series Vulkan engineer delivers.
        void CreateInstance()        {}
        void SelectPhysicalDevice()  {}
        void CreateLogicalDevice()   {}
        void CreateSwapchain()       {}
        void RecreateSwapchain()     {}
        void CreateDepthResources()  {}
    };

} // namespace SZM::Graphics
