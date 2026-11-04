#include "image.h"
#include "device.h"
#include "errorcheck.h"
#include <cassert>
#include <utility>

namespace Lumos
{
    Image::Image(Device& device,
                 uint32_t width,
                 uint32_t height,
                 VkFormat format,
                 VkImageTiling tiling,
                 VkImageUsageFlags usageflags,
                 VkMemoryPropertyFlags properties)
        : m_device(&device)
    {
        VkImageCreateInfo imageinfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {width, height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usageflags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VK_CHECK(vkCreateImage(m_device->device(), &imageinfo, nullptr, &m_image));

        VkMemoryRequirements memrequirements;
        vkGetImageMemoryRequirements(m_device->device(), m_image, &memrequirements);
        VkMemoryAllocateInfo alloinfo {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memrequirements.size,
            .memoryTypeIndex = m_device->findMemoryType(memrequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };
        VK_CHECK(vkAllocateMemory(m_device->device(), &alloinfo, nullptr, &m_memory));
        vkBindImageMemory(m_device->device(), m_image, m_memory, 0);

        //image view
        VkImageViewCreateInfo imageviewinfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
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
        
        VK_CHECK(vkCreateImageView(m_device->device(), &imageviewinfo, nullptr, &m_imageview));
    }

    Image::~Image()
    {
        if (m_imageview != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device->device(), m_imageview, nullptr);
        }
        if (m_image != VK_NULL_HANDLE) {
            vkDestroyImage(m_device->device(), m_image, nullptr);
        }
        if (m_memory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device->device(), m_memory, nullptr);
        }

    }

    Image::Image(Image&& other) noexcept
        : m_device(other.m_device)
        , m_image(std::exchange(other.m_image, VK_NULL_HANDLE))
        , m_memory(std::exchange(other.m_memory, VK_NULL_HANDLE))
        , m_imageview(std::exchange(other.m_imageview, VK_NULL_HANDLE))
    {
    }

    Image& Image::operator=(Image&& other) noexcept
    {
        assert(0 && "TODO: IMPLEMENT THIS");
        if (&other == this) {
            return *this;
        }
        if (m_imageview != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device->device(), m_imageview, nullptr);
        }
        if (m_image != VK_NULL_HANDLE) {
            vkDestroyImage(m_device->device(), m_image, nullptr);
        }
        if (m_memory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device->device(), m_memory, nullptr);
        }

        m_device = std::exchange(other.m_device, nullptr);
        m_image = std::exchange(other.m_image, VK_NULL_HANDLE);
        m_memory = std::exchange(other.m_memory, VK_NULL_HANDLE);
        m_imageview = std::exchange(other.m_imageview, VK_NULL_HANDLE);
        return *this;
    }

    VkImage Image::image() const
    {
        return m_image;
    }

    VkImageView Image::imageview() const
    {
        return m_imageview;
    }
} //namespace Lumos
