#pragma once
#include <vulkan/vulkan.h>

namespace Lumos
{
    class Device;
    class Image
    {
    public:
        Image(Device& device,
              uint32_t width,
              uint32_t height,
              VkFormat format,
              VkImageTiling tiling,
              VkImageUsageFlags usageflags,
              VkMemoryPropertyFlags properties,
              VkImageAspectFlags aspectflags);

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
