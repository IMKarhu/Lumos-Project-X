#include "buffer.h"
#include "device.h"
#include "errorcheck.h"

#include <utility>
#include <cassert>

namespace Lumos
{
    Buffer::Buffer(Device& device, VkDeviceSize buffersize, VkBufferUsageFlags usageflags, VkMemoryPropertyFlags properties)
        : m_device(&device)
        , m_size(buffersize)
    {
        VkBufferCreateInfo bufferinfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = buffersize,
            .usage = usageflags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        VK_CHECK(vkCreateBuffer(m_device->device(), &bufferinfo, nullptr, &m_buffer));

        VkMemoryRequirements memrequirements;
        vkGetBufferMemoryRequirements(m_device->device(), m_buffer, &memrequirements);

        VkMemoryAllocateInfo allocinfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memrequirements.size,
            .memoryTypeIndex = m_device->findMemoryType(memrequirements.memoryTypeBits, properties)
        };

        std::println("------------------INFORMATION START------------------");
        std::println("reguested buffer size: {}", buffersize);
        std::println("memory requirements alignment: {}", memrequirements.alignment);
        std::println("memory requirements size: {}", memrequirements.size);
        std::println("stored buffer size: {}", m_size);
        std::println("------------------INFORMATION END------------------");

        
        VK_CHECK(vkAllocateMemory(m_device->device(), &allocinfo, nullptr, &m_buffermemory));
        VK_CHECK(vkBindBufferMemory(m_device->device(), m_buffer, m_buffermemory, 0));
    }

    Buffer::Buffer(Buffer&& other) noexcept
        : m_device(other.m_device)
        , m_buffer(std::exchange(other.m_buffer, VK_NULL_HANDLE))
        , m_buffermemory(std::exchange(other.m_buffermemory, VK_NULL_HANDLE))
        , m_size(std::exchange(other.m_size, 0))
    {
        other.m_device = nullptr;
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept
    {
        if (&other == this) {
            return *this;
        }
        if (m_buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_device->device(), m_buffer, nullptr);
        }
        if (m_buffermemory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device->device(), m_buffermemory, nullptr);
        }

        m_device = std::exchange(other.m_device, nullptr);
        m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
        m_buffermemory = std::exchange(other.m_buffermemory, VK_NULL_HANDLE);
        m_size = std::exchange(other.m_size, 0);

        return *this;
    }


    Buffer::~Buffer()
    {
        vkDestroyBuffer(m_device->device(), m_buffer, nullptr);
        vkFreeMemory(m_device->device(), m_buffermemory, nullptr);
    }

    VkBuffer Buffer::buffer() const noexcept
    {
        return m_buffer;
    }

    VkDeviceSize Buffer::bufferSize() const noexcept
    {
        return m_size;
    }

    void* Buffer::mappeddata() const noexcept
    {
        return m_mappeddata;
    }


    void* Buffer::map()
    {
        VK_CHECK(vkMapMemory(m_device->device(), m_buffermemory, 0, m_size, 0, &m_mappeddata));
        return m_mappeddata;
    }

    void Buffer::unmap()
    {
        vkUnmapMemory(m_device->device(), m_buffermemory);
    }
} //namespace Lumos
