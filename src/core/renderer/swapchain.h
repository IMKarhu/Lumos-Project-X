#pragma once
#include <vulkan/vulkan.h>
#include "rendererconfig.h"

#include <vector>

namespace Lumos
{
    class Device;
    class Window;

    class SwapChain
    {
    public:
        explicit SwapChain(Device& device, Window& window, const RendererConfig& config = {}, VkSwapchainKHR oldswapchain = VK_NULL_HANDLE);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain operator=(const SwapChain&) = delete;

        VkSwapchainKHR swapchain() const;
        VkFormat imageformat() const;
        VkExtent2D extent() const;
        const std::vector<VkImage>& images() const;
        const std::vector<VkImageView>& imageviews() const;

    private:
        Device& m_device;
        Window& m_window;
        RendererConfig m_config;
        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapchainimages;
        VkFormat m_swapchainimageformat;
        VkExtent2D m_swapchainextent;
        std::vector<VkImageView> m_swapchainimageviews;

        void createSwapchain(VkSwapchainKHR oldswapchain);
        void createImageViews();
        VkSurfaceFormatKHR swapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR swapchainPresentMode(const std::vector<VkPresentModeKHR>& presentmodes);
        VkExtent2D swapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window);
    };
} //namespace Lumos
