#pragma once
#include <vulkan/vulkan.h>

#include <vector>

namespace Lumos
{
    class Device;
    class Window;

    class SwapChain
    {
    public:
        SwapChain(Device& device, Window& window, VkSwapchainKHR oldswapchain = VK_NULL_HANDLE);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain operator=(const SwapChain&) = delete;

        SwapChain(SwapChain&& other) noexcept;
        SwapChain& operator=(SwapChain&& other) noexcept;

        [[nodiscard]] VkSwapchainKHR swapchain() const noexcept;
        [[nodiscard]] VkFormat imageformat() const noexcept;
        [[nodiscard]] VkExtent2D extent() const noexcept;
        [[nodiscard]] std::vector<VkImage> images() const noexcept;
        [[nodiscard]] std::vector<VkImageView> imageviews() const noexcept;

    private:
        Device* m_device = nullptr;
        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapchainimages;
        VkFormat m_swapchainimageformat;
        VkExtent2D m_swapchainextent;
        std::vector<VkImageView> m_swapchainimageviews;

        VkSurfaceFormatKHR swapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR swapchainPresentMode(const std::vector<VkPresentModeKHR>& presentmodes);
        VkExtent2D swapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window);
    };
} //namespace Lumos
