#pragma once
#include <vulkan/vulkan.h>

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
    class Device;
    class CORE_API Image
    {
    public:
        Image(Device& device,
              uint32_t width,
              uint32_t height,
              VkFormat format,
              VkImageTiling tiling,
              VkImageUsageFlags usageflags,
              VkMemoryPropertyFlags properties);

        ~Image();

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        Image(Image&& other) noexcept;
        Image& operator=(Image&& other) noexcept;

        [[nodiscard]] VkImage image() const;
        [[nodiscard]] VkImageView imageview() const;

    private:
        Device* m_device = nullptr;
        VkImageView m_imageview = VK_NULL_HANDLE;
        VkImage m_image = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;
    };
} //namespace Lumos
