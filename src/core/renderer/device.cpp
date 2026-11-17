#include "device.h"
#include "../windowing/window.h"
#include "commandpool.h"
#include "swapchain.h"
#include "buffer.h"
#include "errorcheck.h"
#include "engine/logging.h"

#include <stdexcept>
#include <cstring>
#include <set>

namespace Lumos
{
    static std::vector<const char*> validationlayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    static const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

        ConsoleLogger logger;
        LogLevel level;
        switch(messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                level = LogLevel::Error;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                level = LogLevel::Warning;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
            default:
                level = LogLevel::Info;
        }
        LS_LOG(logger, level, pCallbackData->pMessage);

        return VK_FALSE;
    }


    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createinfo)
    {
        createinfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT This could be useful for debugging? see docs for more information. */
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = nullptr
        };
    }

    VkResult debugUtilsMessenger(VkInstance instance,
                                        const VkDebugUtilsMessengerCreateInfoEXT* pcreateinfo,
                                        const VkAllocationCallbacks* pallocator,
                                        VkDebugUtilsMessengerEXT* pdebugmessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pcreateinfo, pallocator, pdebugmessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void destroyDebugUtilsMessenger(VkInstance instance,
                                           VkDebugUtilsMessengerEXT debugmessenger,
                                           const VkAllocationCallbacks* pallocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugmessenger, pallocator);
        }
    }


    Device::Device(Window& window, const RendererConfig config)
        : m_window(window)
        , m_config(config)
    {
        createInstance();
        if (config.validationlayers) {
            createDebugMessenger();
        }
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
    }

    Device::~Device()
    {
        std::println("device destructor called");
        destroyDebugUtilsMessenger(m_instance, m_debugmessenger, nullptr);
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }
        
    VkDevice Device::device() const
    {
        return m_device;
    }

    VkPhysicalDevice Device::pdevice() const
    {
        return m_physicaldevice;
    }

    VkSurfaceKHR Device::surface() const
    {
        return m_surface;
    }

    VkQueue Device::graphicsQueue() const
    {
        return m_graphicsqueue;
    }

    VkQueue Device::presentQueue() const
    {
        return m_presentqueue;
    }

    QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice pdevice)
    {
        QueueFamilyIndices indices;

        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &familyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &familyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queFamily : queueFamilies)
        {
            if (queFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(pdevice, i, m_surface, &presentSupport);

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }
            i++;
        }

        return indices;

    }

    SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice pdevice)
    {
        SwapChainSupportDetails details;

        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdevice, m_surface, &details.capabilities));

        uint32_t formaCount;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, m_surface, &formaCount, nullptr));

        if (formaCount != 0)
        {
            details.formats.resize(formaCount);
            VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(pdevice, m_surface, &formaCount, details.formats.data()));
        }

        uint32_t presentModeCount;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, m_surface, &presentModeCount, nullptr));

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(pdevice, m_surface, &presentModeCount, details.presentModes.data()));
        }

        return details;
    }


    uint32_t Device::findMemoryType(uint32_t typefilter, VkMemoryPropertyFlags flags)
    {
        VkPhysicalDeviceMemoryProperties memproperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicaldevice, &memproperties);

        for (size_t i = 0; i < memproperties.memoryTypeCount; i++)
        {
            if ((typefilter & (1 << i)) && (memproperties.memoryTypes[i].propertyFlags & flags) == flags)
            {
                return i;
            }
        }
        throw std::runtime_error("could not find memory type\n");
        return 0;
    }
    //
    // VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& formats,
    //                                      VkImageTiling tiling,
    //                                      VkFormatFeatureFlags flags)
    // {
    //     for (const auto format : formats) {
    //         VkFormatProperties properties;
    //         vkGetPhysicalDeviceFormatProperties(m_physicaldevice, format, &properties);
    //         if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & flags) == flags) {
    //             return format;
    //         } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & flags) == flags) {
    //             return format;
    //         }
    //     }
    //     throw std::runtime_error("failed to find supported format");
    // }
    //
    // VkFormat Device::depthformat()
    // {
    //     return findSupportedFormat(
    //             {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    //              VK_IMAGE_TILING_OPTIMAL,
    //              VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    // }
    //
    // bool Device::stenchilComponent(VkFormat format)
    // {
    //     return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    // }
    //
    //
    // void Device::copyBuffers(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size)
    // {
    //     VkCommandBuffer cmdbuf = beginSingleTimeCommands();
    //     VkBufferCopy copyregion {
    //         .srcOffset = 0,
    //             .dstOffset = 0,
    //             .size = size
    //     };
    //     vkCmdCopyBuffer(cmdbuf, srcBuffer.buffer(), dstBuffer.buffer(), 1, &copyregion);
    //     endSingleTimeCommands(cmdbuf);
    // }
    //
    // void Device::copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height)
    // {
    //     VkCommandBuffer cmdbuf = beginSingleTimeCommands();
    //     VkBufferImageCopy region {
    //         .bufferOffset = 0,
    //         .bufferRowLength = 0,
    //         .bufferImageHeight = 0,
    //         .imageSubresource {
    //             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //             .mipLevel = 0,
    //             .baseArrayLayer = 0,
    //             .layerCount = 1
    //         },
    //         .imageOffset = {0, 0, 0},
    //         .imageExtent = {width, height, 1}
    //     };
    //     vkCmdCopyBufferToImage(cmdbuf,
    //                            buffer.buffer(),
    //                            image,
    //                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                            1,
    //                            &region);
    //     endSingleTimeCommands(cmdbuf);
    // }
    //
    // void Device::transitionImageLayout(VkImage image,
    //                                    VkCommandBuffer cmdbuf,
    //                                    VkImageLayout oldlayout,
    //                                    VkImageLayout newlayout,
    //                                    VkPipelineStageFlags2 srcstagemask,
    //                                    VkPipelineStageFlags2 dststagemask,
    //                                    VkAccessFlags2 srcaccessmask,
    //                                    VkAccessFlags2 dstaccessmask,
    //                                    VkImageAspectFlags aspectflags)
    // {
    //     VkImageMemoryBarrier2 barrier {
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //         .srcStageMask = srcstagemask,
    //         .srcAccessMask = srcaccessmask,
    //         .dstStageMask = dststagemask,
    //         .dstAccessMask = dstaccessmask,
    //         .oldLayout = oldlayout,
    //         .newLayout = newlayout,
    //         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //         .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //         .image = image,
    //         .subresourceRange = {
    //             .aspectMask = aspectflags,
    //             .baseMipLevel = 0,
    //             .levelCount = 1,
    //             .baseArrayLayer = 0,
    //             .layerCount = 1
    //         }
    //     };
    //     VkDependencyInfo dependecyinfo {
    //         .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //         .dependencyFlags = {},
    //         .imageMemoryBarrierCount = 1,
    //         .pImageMemoryBarriers = &barrier
    //     };
    //
    //     vkCmdPipelineBarrier2(cmdbuf, &dependecyinfo);
    // }
    //
    // void Device::transitionImageLayout2(VkImage image,
    //                                     VkFormat format,
    //                                     VkImageLayout oldlayout,
    //                                     VkImageLayout newlayout)
    // {
    //     VkCommandBuffer cmdbuf = beginSingleTimeCommands();
    //     VkImageMemoryBarrier barrier {
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    //         .srcAccessMask = 0,
    //         .dstAccessMask = 0,
    //         .oldLayout = oldlayout,
    //         .newLayout = newlayout,
    //         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //         .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //         .image = image,
    //         .subresourceRange {
    //             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //             .baseMipLevel = 0,
    //             .levelCount = 1,
    //             .baseArrayLayer = 0,
    //             .layerCount = 1,
    //         },
    //     };
    //     VkPipelineStageFlags sourcestage;
    //     VkPipelineStageFlags destinationstage;
    //     if (oldlayout == VK_IMAGE_LAYOUT_UNDEFINED &&
    //         newlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    //         barrier.srcAccessMask = 0;
    //         barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //         sourcestage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //         destinationstage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    //     } else if (oldlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
    //                newlayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    //         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //
    //         sourcestage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    //         destinationstage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //     } else {
    //         throw std::invalid_argument("unsupported layout transition!");
    //     }
    //     vkCmdPipelineBarrier(cmdbuf,
    //                          sourcestage, destinationstage,
    //                          0,
    //                          0, nullptr,
    //                          0, nullptr,
    //                          1, &barrier);
    //     endSingleTimeCommands(cmdbuf);
    // }
    //
    // void Device::setFramebufferResized(bool resized)
    // {
    //     m_framebufferresized = resized;
    // }
    //
    //
    //
    /****** PRIVATE ******/

    bool Device::createInstance()
    {
         if (m_config.validationlayers && !checkValidationSupport()) {
             throw std::runtime_error("No validation layers are active");
         }

         VkApplicationInfo appinfo {
             .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
             .pApplicationName = m_config.name.c_str(),
             .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
             .pEngineName = "Lumos",
             .engineVersion = VK_MAKE_VERSION(1, 0, 0),
             .apiVersion = VK_API_VERSION_1_4
         };

         VkInstanceCreateInfo instanceinfo {
             .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
             .pApplicationInfo = &appinfo
         };
         std::vector<const char*> instanceextensions = m_window.instanceExtensions();

         if (m_config.validationlayers) {
             instanceextensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
         }
         instanceinfo.enabledExtensionCount = static_cast<uint32_t>(instanceextensions.size());
         instanceinfo.ppEnabledExtensionNames = instanceextensions.data();
         VkDebugUtilsMessengerCreateInfoEXT debuginfo{};
         if (m_config.validationlayers) {
             instanceinfo.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
             instanceinfo.ppEnabledLayerNames = validationlayers.data();
             populateDebugMessengerCreateInfo(debuginfo);
             instanceinfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debuginfo;
         }
         else
         {
             instanceinfo.enabledLayerCount = 0;
             instanceinfo.pNext = nullptr;
         }

         VK_CHECK(vkCreateInstance(&instanceinfo, nullptr, &m_instance));
         return true;
    }

    bool Device::createSurface()
    {
        VK_CHECK(glfwCreateWindowSurface(m_instance, m_window.handle(), nullptr, &m_surface));
        return true;
    }

    void Device::pickPhysicalDevice()
    {
        uint32_t count = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, nullptr));
        if (count == 0) {
            throw std::runtime_error("Failed to find physical device with vulkan support");
        }
        std::vector<VkPhysicalDevice> pdevices(count);
        VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &count, pdevices.data()));
        for (const auto& device : pdevices) {
            if (deviceSuitable(device)) {
                m_physicaldevice = device;
                break;
            }
        }
        if (m_physicaldevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find suitable graphics device");
        }
    }

    void Device::createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(m_physicaldevice);
        std::vector<VkDeviceQueueCreateInfo> queinfo{};
        std::set<uint32_t> uniquefamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        for (uint32_t family : uniquefamilies) {
            float quepriority = 1.0f;
            VkDeviceQueueCreateInfo deviceinfo {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = family,
                .queueCount = 1,
                .pQueuePriorities = &quepriority
            };
            queinfo.push_back(deviceinfo);
        }
        VkPhysicalDeviceFeatures devicefeatures {
            .samplerAnisotropy = VK_TRUE
        };
        VkPhysicalDeviceVulkan11Features features11 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .shaderDrawParameters = VK_TRUE,
        };
        VkPhysicalDeviceSynchronization2Features sync2features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
            .pNext = &features11,
            .synchronization2 = VK_TRUE
        };
        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicrenderfeature {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = &sync2features,
            .dynamicRendering = VK_TRUE
        };

        VkDeviceCreateInfo deviceinfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &dynamicrenderfeature,
            .queueCreateInfoCount = static_cast<uint32_t>(uniquefamilies.size()),
            .pQueueCreateInfos =queinfo.data(),
            .enabledLayerCount = 0,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &devicefeatures
        };
        if (m_config.validationlayers) {
            deviceinfo.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
            deviceinfo.ppEnabledLayerNames = validationlayers.data();
        }
        VK_CHECK(vkCreateDevice(m_physicaldevice, &deviceinfo, nullptr, &m_device));
        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsqueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentqueue);

    }

    bool Device::deviceSuitable(VkPhysicalDevice pdevice)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(pdevice, &deviceProperties);

        QueueFamilyIndices indices = findQueueFamilies(pdevice);

        bool extensionsSupported = checkDeviceExtensionSupport(pdevice);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pdevice);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(pdevice, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool Device::checkDeviceExtensionSupport(VkPhysicalDevice pdevice)
    {
        uint32_t deviceExtensionCount;
        vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &deviceExtensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(deviceExtensionCount);
        vkEnumerateDeviceExtensionProperties(pdevice, nullptr, &deviceExtensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    bool Device::checkValidationSupport() const
    {
        uint32_t count;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&count, nullptr));
        std::vector<VkLayerProperties> layers(count);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&count, layers.data()));
        for (const char* vallayer : validationlayers) {
            bool found = false;
            for (const auto& layer : layers) {
                std::println("layer name: {}", layer.layerName);
                if (strcmp(vallayer, layer.layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;

    }

    void Device::createDebugMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT debuginfo{};
        populateDebugMessengerCreateInfo(debuginfo);
        VK_CHECK(debugUtilsMessenger(m_instance, &debuginfo, nullptr, &m_debugmessenger));
    }
} //namespace Lumos
