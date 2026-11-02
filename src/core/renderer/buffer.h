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
    class CORE_API Buffer
    {
    public:
        Buffer(Device& device,
               VkDeviceSize buffersize,
               VkBufferUsageFlags usageflags,
               VkMemoryPropertyFlags properties);

        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        [[nodiscard]] VkBuffer buffer() const noexcept;
        [[nodiscard]] VkDeviceSize bufferSize() const noexcept;
        [[nodiscard]] void* mappeddata() const noexcept;

        void* map();
        void unmap();
    private:
        Device* m_device = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_buffermemory = VK_NULL_HANDLE;
        VkDeviceSize m_size = 0;
        void* m_mappeddata = nullptr;
    };
} //namespace Lumos
