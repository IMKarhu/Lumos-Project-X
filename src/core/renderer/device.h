#pragma once
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "rendererconfig.h"
namespace Lumos
{

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class Window;
    class Buffer;
    class Device
    {
    public:

        explicit Device(Window& window, const RendererConfig config = {});
        ~Device();

        Device(const Device&) = delete;
        Device operator=(const Device&) = delete;

        VkDevice device() const;
        VkPhysicalDevice pdevice() const;
        VkSurfaceKHR surface() const;
        VkQueue graphicsQueue() const;
        VkQueue presentQueue() const;
        bool initialized() const;

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pdevice);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice pdevice);
        uint32_t findMemoryType(uint32_t typefilter, VkMemoryPropertyFlags flags);

        // VkFormat findSupportedFormat(const std::vector<VkFormat>& formats,
        //                              VkImageTiling tiling,
        //                              VkFormatFeatureFlags flags);
        // VkFormat depthformat();
        // bool stenchilComponent(VkFormat format);
        // VkCommandBuffer beginSingleTimeCommands();
        // void endSingleTimeCommands(VkCommandBuffer cmdbuf);
        // void copyBuffers(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size);
        // void copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height);
        // void transitionImageLayout(VkImage image,
        //                            VkCommandBuffer cmdbuf,
        //                            VkImageLayout oldlayout,
        //                            VkImageLayout newlayout,
        //                            VkPipelineStageFlags2 srcstagemask,
        //                            VkPipelineStageFlags2 dststagemask,
        //                            VkAccessFlags2 srcaccessmask,
        //                            VkAccessFlags2 dstaccessmask,
        //                            VkImageAspectFlags aspectflags);
        // void transitionImageLayout2(VkImage image,
        //                             VkFormat format,
        //                             VkImageLayout oldlayout,
        //                             VkImageLayout newlayout);
    private:
        Window& m_window;
        bool createInstance();
        bool createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        bool deviceSuitable(VkPhysicalDevice pdevice);

        RendererConfig m_config;
        bool m_instaninitialized = false;
        bool m_surfaceinitialized = false;
        bool deviceinitialized = false;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicaldevice = VK_NULL_HANDLE;
        VkQueue m_graphicsqueue = VK_NULL_HANDLE;
        VkQueue m_presentqueue = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugmessenger = VK_NULL_HANDLE;
        bool checkDeviceExtensionSupport(VkPhysicalDevice pdevice);
        bool checkValidationSupport() const;
        void createDebugMessenger();


    };
} //namespace Lumos
