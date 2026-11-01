#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Lumos
{
    class Device;
    class CommandPool
    {
    public:
        CommandPool(Device& device, uint32_t queuefamilyindex);
        ~CommandPool();

        VkCommandPool pool() const noexcept;
        std::vector<VkCommandBuffer> alloc(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        void reset();
    private:
        Device* m_device = nullptr;
        VkCommandPool m_pool = VK_NULL_HANDLE;
    };
} //namespace Lumos
