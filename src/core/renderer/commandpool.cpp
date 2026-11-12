#include "commandpool.h"
#include "errorcheck.h"
#include "renderer.h"

namespace Lumos
{
    CommandPool::CommandPool(Renderer* renderer, uint32_t queuefamilyindex)
        :m_renderer(renderer)
    {
        // VkCommandPoolCreateInfo poolinfo{};
        // poolinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // poolinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // poolinfo.queueFamilyIndex = queuefamilyindex;
        // VK_CHECK(vkCreateCommandPool(renderer->device(), &poolinfo, nullptr, &m_pool));
    }

    CommandPool::~CommandPool()
    {

    }

    VkCommandPool CommandPool::pool() const noexcept
    {
        return m_pool;
    }

    std::vector<VkCommandBuffer> CommandPool::alloc(uint32_t count, VkCommandBufferLevel level)
    {
        // std::vector<VkCommandBuffer> buffers(count);
        // VkCommandBufferAllocateInfo allocinfo{};
        // allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        // allocinfo.commandPool = m_pool;
        // allocinfo.level = level;
        // allocinfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
        // VK_CHECK(vkAllocateCommandBuffers(m_renderer->device(), &allocinfo, buffers.data()));
        // return buffers;
    }

    void CommandPool::reset()
    {
        vkResetCommandPool(m_renderer->device(), m_pool, 0);
    }
} //namespace Lumos
