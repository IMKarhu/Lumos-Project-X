#pragma once
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>
#include <memory>

#ifdef _WIN32
    #ifdef CORE_EXPORT
    #define CORE_API __declspec(dllexport)
    #else
    #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API
#endif

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

    struct VulkanConfig
    {
        uint32_t framesinflight = 2;
        bool validationlayers = false;
    };


    class Window;
    class SwapChain;
    class CommandPool;
    class Buffer;
    class CORE_API Device
    {
    public:
        struct FrameInfo
        {
            uint32_t imageindex;
            uint32_t frameindex;
            VkCommandBuffer cmdbuf;
            //currently just for swapchain usage
            VkImage image;
            VkImageView imageview;
            VkExtent2D extent;
        };

        Device(Window& window, VulkanConfig config);
        ~Device();

        Device(const Device&) = delete;
        Device operator=(const Device&) = delete;

        Device(Device&& other) noexcept;
        Device& operator=(Device&& other) noexcept;

        FrameInfo startFrame();
        void endFrame(FrameInfo frameinfo);
        void waitForDestroy();
        [[nodiscard]] VkDevice device() const;
        [[nodiscard]] VkPhysicalDevice pdevice() const;
        [[nodiscard]] VkSurfaceKHR surface() const;
        [[nodiscard]] VkQueue graphqueue() const;
        [[nodiscard]] VkQueue presentqueue() const;
        [[nodiscard]] VkFormat defaultformat() const;

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pdevice);
        uint32_t findMemoryType(uint32_t typefilter, VkMemoryPropertyFlags flags);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice pdevice);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer cmdbuf);
        void copyBuffers(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size);
        void copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height);
        void transitionImageLayout(VkImage image,
                                   VkCommandBuffer cmdbuf,
                                   VkImageLayout oldlayout,
                                   VkImageLayout newlayout,
                                   VkPipelineStageFlags2 srcstagemask,
                                   VkPipelineStageFlags2 dststagemask,
                                   VkAccessFlags2 srcaccessmask,
                                   VkAccessFlags2 dstaccessmask);
        void transitionImageLayout2(VkImage image,
                                    VkFormat format,
                                    VkImageLayout oldlayout,
                                    VkImageLayout newlayout);



    private:
        Window& m_window;
        bool m_validationlayer = false;
        uint32_t m_maxframesinflight = 2;
        uint32_t frameindex = 0;
        VkDebugUtilsMessengerEXT m_debugmessenger = VK_NULL_HANDLE;
        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicaldevice = VK_NULL_HANDLE;
        VkQueue m_graphicsqueue = VK_NULL_HANDLE;
        VkQueue m_presentqueue = VK_NULL_HANDLE;

        std::unique_ptr<SwapChain> m_swapchain;
        std::unique_ptr<CommandPool> m_commandpool;
        std::vector<VkCommandBuffer> m_commandbuffers;

        std::vector<VkSemaphore> m_presentSemaphores;
        std::vector<VkSemaphore> m_finishedSemaphores;
        std::vector<VkFence> m_fences;


        bool deviceSuitable(VkPhysicalDevice pdevice);
        bool checkDeviceExtensionSupport(VkPhysicalDevice pdevice);
        bool checkValidationSupport();
        std::vector<const char*> requiredExtensions();
        void setupDebugging();
        void recreateSwapchain();
    };
} //namespace Lumos
