#include "device.h"
#include "utils.h"
#include "../windowing/window.h"
#include "commandpool.h"
#include "swapchain.h"
#include "buffer.h"
#include "errorcheck.h"

#include <stdexcept>
#include <cstring>
#include <set>

namespace Lumos
{
    Device::Device(Window& window, VulkanConfig config)
        : m_window(window)
    {
        m_validationlayer = config.validationlayers;
        m_maxframesinflight = config.framesinflight;
        if (m_validationlayer && !checkValidationSupport()) {
            throw std::runtime_error("No validation layers are active");
        }

        VkApplicationInfo appinfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "LumosV2",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Lumos",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_4
        };

        VkInstanceCreateInfo instanceinfo {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appinfo
        };

        auto extensions = requiredExtensions();
        instanceinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceinfo.ppEnabledExtensionNames = extensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debuginfo{};
        if (m_validationlayer) {
            instanceinfo.enabledLayerCount = static_cast<uint32_t>(utils::validationlayers.size());
            instanceinfo.ppEnabledLayerNames = utils::validationlayers.data();
            utils::populateDebugMessengerCreateInfo(debuginfo);
            instanceinfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debuginfo;
        }
        else
        {
            instanceinfo.enabledLayerCount = 0;
            instanceinfo.pNext = nullptr;
        }

        VK_CHECK(vkCreateInstance(&instanceinfo, nullptr, &m_instance));

        glfwCreateWindowSurface(m_instance, window.handle(), nullptr, &m_surface);

        //picking physical device
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

        //Logical device
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
            .enabledExtensionCount = static_cast<uint32_t>(utils::deviceExtensions.size()),
            .ppEnabledExtensionNames = utils::deviceExtensions.data(),
            .pEnabledFeatures = &devicefeatures
        };
        if (m_validationlayer) {
            deviceinfo.enabledLayerCount = static_cast<uint32_t>(utils::validationlayers.size());
            deviceinfo.ppEnabledLayerNames = utils::validationlayers.data();
        }
        VK_CHECK(vkCreateDevice(m_physicaldevice, &deviceinfo, nullptr, &m_device));
        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsqueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentqueue);

        if (m_validationlayer) {
            setupDebugging();
        }

        //commandbuffers and pool
        // QueueFamilyIndices indices = m_device->findQueueFamilies(m_device->pdevice());
        m_commandpool = std::make_unique<CommandPool>(*this, indices.graphicsFamily.value());
        m_commandbuffers = m_commandpool->alloc(m_maxframesinflight);

        //swapchain
        m_swapchain = std::make_unique<SwapChain>(*this, window);

        //sync stuff
        m_finishedSemaphores.resize(m_swapchain->images().size());
        m_presentSemaphores.resize(m_swapchain->images().size());
        m_fences.resize(m_swapchain->images().size());
        
        VkSemaphoreCreateInfo semaphoreinfo{};
        semaphoreinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceinfo{};
        fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        std::println("size of images: {}",m_swapchain->images().size()); 

        for (size_t i = 0; i < m_swapchain->images().size(); i++) {
            VK_CHECK(vkCreateSemaphore(m_device, &semaphoreinfo, nullptr, &m_finishedSemaphores[i]));
            VK_CHECK(vkCreateSemaphore(m_device, &semaphoreinfo, nullptr, &m_presentSemaphores[i]));
            VK_CHECK(vkCreateFence(m_device, &fenceinfo, nullptr, &m_fences[i]));
        }

    }

    Device::~Device()
    {

    }

    /*
     * Brief: Acquires next swapchain image.
     * reset commandbuffer.
     * begins new commandbuffer.
     * fills FrameInfo struct and returns it.
     */
    Device::FrameInfo Device::startFrame()
    {
        FrameInfo frameinfo;
        VK_CHECK(vkWaitForFences(m_device, 1, &m_fences[frameindex], VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(m_device, 1, &m_fences[frameindex]));
        VkResult result = vkAcquireNextImageKHR(m_device,
                                                m_swapchain->swapchain(),
                                                UINT64_MAX,
                                                m_presentSemaphores[frameindex],
                                                VK_NULL_HANDLE,
                                                &frameinfo.imageindex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            std::println("swapchain out of date\n");
            recreateSwapchain();
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to get swapchain image!\n");
        }

        frameinfo.cmdbuf = m_commandbuffers[frameindex];
        VK_CHECK(vkResetCommandBuffer(frameinfo.cmdbuf, 0));
        VkCommandBufferBeginInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(m_commandbuffers[frameindex], &info));
        frameinfo.image = m_swapchain->images()[frameinfo.imageindex];
        frameinfo.imageview = m_swapchain->imageviews()[frameinfo.imageindex];
        frameinfo.extent = m_swapchain->extent();
        frameinfo.frameindex = frameindex;
        return frameinfo;
    }

    /*
     * Brief: Ends command buffer recording.
     * Submits commands to queue.
     */
    void Device::endFrame(FrameInfo frameinfo)
    {
        VK_CHECK(vkEndCommandBuffer(frameinfo.cmdbuf));

        VkSubmitInfo submitinfo{};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_presentSemaphores[frameindex] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &frameinfo.cmdbuf;
        VkSemaphore signalSemaphores[] = { m_finishedSemaphores[frameindex] };
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK(vkQueueSubmit(m_graphicsqueue, 1, &submitinfo, m_fences[frameindex]));

        VkPresentInfoKHR presentinfo{};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_swapchain->swapchain() };
        presentinfo.swapchainCount = 1;
        presentinfo.pSwapchains = swapChains;
        presentinfo.pImageIndices = &frameinfo.imageindex;
        presentinfo.pResults = nullptr;

        VkResult res = vkQueuePresentKHR(m_presentqueue, &presentinfo);
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_window.framebufferResized()) {
            std::println("swapchain out of date, suboptimal or framebuffer was resized\n");
            m_window.setFramebufferResized(false);
            recreateSwapchain();
        }
        else if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to present swapchain image");
        }
        frameindex = (frameindex + 1) % m_maxframesinflight;
        // std::println("frameindex: {}", frameindex);
    }

    void Device::waitForDestroy()
    {
        VK_CHECK(vkDeviceWaitIdle(m_device));
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

    VkQueue Device::graphqueue() const
    {
        return m_graphicsqueue;
    }
    
    VkQueue Device::presentqueue() const
    {
        return m_presentqueue;
    }

    VkFormat Device::defaultformat() const
    {
        return m_swapchain->imageformat();
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

    VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& formats,
                                         VkImageTiling tiling,
                                         VkFormatFeatureFlags flags)
    {
        for (const auto format : formats) {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(m_physicaldevice, format, &properties);
            if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & flags) == flags) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & flags) == flags) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format");
    }

    VkFormat Device::depthformat()
    {
        return findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    bool Device::stenchilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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

    VkCommandBuffer Device::beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocinfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_commandpool->pool(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        VkCommandBuffer cmdbuffer;
        VK_CHECK(vkAllocateCommandBuffers(m_device, &allocinfo, &cmdbuffer));

        VkCommandBufferBeginInfo begininfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        VK_CHECK(vkBeginCommandBuffer(cmdbuffer, &begininfo));

        return cmdbuffer;
    }

    void Device::endSingleTimeCommands(VkCommandBuffer cmdbuf)
    {
        vkEndCommandBuffer(cmdbuf);
        VkSubmitInfo submitinfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdbuf
        };

        VK_CHECK(vkQueueSubmit(m_presentqueue, 1, &submitinfo, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(m_presentqueue));

        vkFreeCommandBuffers(m_device, m_commandpool->pool(), 1, &cmdbuf);
    }

    void Device::copyBuffers(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer cmdbuf = beginSingleTimeCommands();
        VkBufferCopy copyregion {
            .srcOffset = 0,
                .dstOffset = 0,
                .size = size
        };
        vkCmdCopyBuffer(cmdbuf, srcBuffer.buffer(), dstBuffer.buffer(), 1, &copyregion);
        endSingleTimeCommands(cmdbuf);
    }

    void Device::copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer cmdbuf = beginSingleTimeCommands();
        VkBufferImageCopy region {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {width, height, 1}
        };
        vkCmdCopyBufferToImage(cmdbuf,
                               buffer.buffer(),
                               image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);
        endSingleTimeCommands(cmdbuf);
    }

    void Device::transitionImageLayout(VkImage image,
                                       VkCommandBuffer cmdbuf,
                                       VkImageLayout oldlayout,
                                       VkImageLayout newlayout,
                                       VkPipelineStageFlags2 srcstagemask,
                                       VkPipelineStageFlags2 dststagemask,
                                       VkAccessFlags2 srcaccessmask,
                                       VkAccessFlags2 dstaccessmask,
                                       VkImageAspectFlags aspectflags)
    {
        VkImageMemoryBarrier2 barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = srcstagemask,
            .srcAccessMask = srcaccessmask,
            .dstStageMask = dststagemask,
            .dstAccessMask = dstaccessmask,
            .oldLayout = oldlayout,
            .newLayout = newlayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = aspectflags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VkDependencyInfo dependecyinfo {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        vkCmdPipelineBarrier2(cmdbuf, &dependecyinfo);
    }

    void Device::transitionImageLayout2(VkImage image,
                                        VkFormat format,
                                        VkImageLayout oldlayout,
                                        VkImageLayout newlayout)
    {
        VkCommandBuffer cmdbuf = beginSingleTimeCommands();
        VkImageMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = oldlayout,
            .newLayout = newlayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        VkPipelineStageFlags sourcestage;
        VkPipelineStageFlags destinationstage;
        if (oldlayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourcestage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationstage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newlayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourcestage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationstage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }
        vkCmdPipelineBarrier(cmdbuf,
                             sourcestage, destinationstage,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        endSingleTimeCommands(cmdbuf);
    }



    /****** PRIVATE ******/
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

        std::set<std::string> requiredExtensions(utils::deviceExtensions.begin(), utils::deviceExtensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    bool Device::checkValidationSupport()
    {
        uint32_t count;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&count, nullptr));
        std::vector<VkLayerProperties> layers(count);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&count, layers.data()));
        for (const char* vallayer : utils::validationlayers) {
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

    std::vector<const char*> Device::requiredExtensions()
    {
        uint32_t count = 0;
        const char** glfwextensions;
        glfwextensions = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char*> extensions(glfwextensions, glfwextensions + count);
        if (m_validationlayer) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;

    }

    void Device::setupDebugging()
    {
        VkDebugUtilsMessengerCreateInfoEXT debuginfo{};
        utils::populateDebugMessengerCreateInfo(debuginfo);
        VK_CHECK(utils::debugUtilsMessenger(m_instance, &debuginfo, nullptr, &m_debugmessenger));
    }
 
    //TODO: FIX THIS ASAP this does not handle DEPTH at the moment
    void Device::recreateSwapchain()
    {
        int width = 0;
        int height = 0;
        m_window.framebufferSize(width, height);
        if (width == 0 || height == 0) {
            if (m_window.shouldClose()) {
                return;
            }
            m_window.framebufferSize(width, height);
            m_window.waitEvents();
        }
        VK_CHECK(vkDeviceWaitIdle(m_device));
        auto replacement = std::make_unique<SwapChain>(*this, m_window, m_swapchain->swapchain());
        m_swapchain = std::move(replacement);
    }

} //namespace Lumos
