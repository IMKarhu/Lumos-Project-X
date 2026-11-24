#include "swapchain.h"
#include "device.h"
#include "../windowing/window.h"
#include "engine/logging.h"
#include "errorcheck.h"

#include <print>
#include <algorithm>
#include <utility>

namespace Lumos
{
    SwapChain::SwapChain(Device& device, Window& window, const RendererConfig& config, VkSwapchainKHR oldswapchain)
        : m_device(device)
        , m_window(window)
        , m_config(config)
    {
        createSwapchain(oldswapchain);
        createImageViews();
    }

    SwapChain::~SwapChain()
    {
        for (auto view : m_swapchainimageviews)
        {
            vkDestroyImageView(m_device.device(), view, nullptr);
        }
        vkDestroySwapchainKHR(m_device.device(), m_swapchain, nullptr);
    }

    VkSwapchainKHR SwapChain::swapchain() const
    {
        return m_swapchain;
    }

    VkFormat SwapChain::imageformat() const
    {
        return m_swapchainimageformat;
    }

    VkExtent2D SwapChain::extent() const
    {
        return m_swapchainextent;
    }

    const std::vector<VkImage>& SwapChain::images() const
    {
        return m_swapchainimages;
    }
    const std::vector<VkImageView>& SwapChain::imageviews() const
    {
        return m_swapchainimageviews;
    }


    /****** PRIVATE ******/

    void SwapChain::createSwapchain(VkSwapchainKHR oldswapchain)
    {
        SwapChainSupportDetails swapchainsupport = m_device.querySwapChainSupport(m_device.pdevice());
        VkSurfaceFormatKHR surfaceformat = swapchainSurfaceFormat(swapchainsupport.formats);
        VkPresentModeKHR presentmode = swapchainPresentMode(swapchainsupport.presentModes);
        VkExtent2D extent = swapExtent(swapchainsupport.capabilities, m_window);

        uint32_t imagecount = swapchainsupport.capabilities.minImageCount + 1;
        if (swapchainsupport.capabilities.maxImageCount > 0
            && imagecount > swapchainsupport.capabilities.maxImageCount) {
            imagecount = swapchainsupport.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR swapinfo {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_device.surface(),
            .minImageCount = imagecount,
            .imageFormat = surfaceformat.format,
            .imageColorSpace = surfaceformat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        };
        std::println("swapchain minImageCount {}", swapinfo.minImageCount);

        auto indices = m_device.findQueueFamilies(m_device.pdevice());
        uint32_t queuefamilyindices[] = {indices.graphicsFamily.value(), indices.presentFamily.value() };
        if (indices.graphicsFamily != indices.presentFamily) {
            swapinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapinfo.queueFamilyIndexCount = 2;
            swapinfo.pQueueFamilyIndices = queuefamilyindices;
        }
        else
        {
            swapinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapinfo.queueFamilyIndexCount = 0;
            swapinfo.pQueueFamilyIndices = nullptr;
        }

        swapinfo.preTransform = swapchainsupport.capabilities.currentTransform;
        swapinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapinfo.presentMode = presentmode;
        swapinfo.clipped = VK_TRUE;
        swapinfo.oldSwapchain = oldswapchain;
        
        VK_CHECK(vkCreateSwapchainKHR(m_device.device(), &swapinfo, nullptr, &m_swapchain));
        
        VK_CHECK(vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &imagecount, nullptr));
        m_swapchainimages.resize(imagecount);
        VK_CHECK(vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &imagecount, m_swapchainimages.data()));
        m_swapchainimageformat = surfaceformat.format;
        m_swapchainextent = extent;
    }

    void SwapChain::createImageViews()
    {
        //image views
        m_swapchainimageviews.resize(m_swapchainimages.size());
        for (size_t i = 0; i < m_swapchainimageviews.size(); i++) {
            //Probably a good idea to refactor this into its own function at some point,
            //because we are going to need to do this other places as well
            //and would just be code duplication at that point
            VkImageViewCreateInfo imageviewinfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_swapchainimages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_swapchainimageformat,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };
            
            VK_CHECK(vkCreateImageView(m_device.device(), &imageviewinfo, nullptr, &m_swapchainimageviews[i]));
        }
    }

    VkSurfaceFormatKHR SwapChain::swapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        ConsoleLogger logger;
        for (auto& format : std::as_const(formats))
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                LS_LOG_INFO(logger,"SwapChain surface format: VK_FORMAT_B8G8R8A8_SRGB and colorSpace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR");
                return format;
            }
        }
        return formats[0];
    }

    VkPresentModeKHR SwapChain::swapchainPresentMode(const std::vector<VkPresentModeKHR>& presentmodes)
    {
        ConsoleLogger logger;
        if (!m_config.vsync) {
            for (auto& presentMode : std::as_const(presentmodes))
            {
                if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    LS_LOG_INFO(logger, "using VK_PRESENT_MODE_MAILBOX_KHR");
                    return presentMode;
                }
            }

            //maybe fallback to immediated mode?
            for(auto& presentMode : std::as_const(presentmodes)) {
                if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                    LS_LOG_INFO(logger, "using VK_PRESENT_MODE_IMMEDIATE_KHR");
                    return presentMode;
                }
            }
        }
        LS_LOG_INFO(logger, "using VK_PRESENT_MODE_FIFO_KHR");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::swapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window.handle(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
} //namespace Lumos
