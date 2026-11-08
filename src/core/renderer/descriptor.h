#pragma once
#include <vulkan/vulkan.h>
#include <span>
#include <vector>
#include <unordered_map>

namespace Lumos
{

    enum class DescriptorType
    {
        Uniform,
        Storage,
        CombinedImageSampler
    };
    enum class StageFlags
    {
        Vertex,
        Fragment
    };

    struct BindingConfig
    {
        uint32_t binding;
        DescriptorType type;
        uint32_t count;
        StageFlags flags;
        //IS MISSING IMMUTABLE SAMPLER
    };

    struct PoolConfig
    {
        uint32_t count;
        DescriptorType type;
    };

    class Device;
    class DescriptorLayout
    {
    public:
        DescriptorLayout(Device& device, std::span<const BindingConfig> bindings);
        ~DescriptorLayout();

        DescriptorLayout(const DescriptorLayout&) = delete;
        DescriptorLayout& operator=(const DescriptorLayout&) = delete;

        DescriptorLayout(DescriptorLayout&& other) noexcept;
        DescriptorLayout& operator=(DescriptorLayout&& other) noexcept;

        [[nodiscard]] VkDescriptorSetLayout layout() const noexcept;
    private:
        Device* m_device = nullptr;
        VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    };

    class DescriptorPool
    {
    public:
        DescriptorPool(Device& device, std::span<const PoolConfig> poolsizes, uint32_t maxsets);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        DescriptorPool(DescriptorPool&& other) noexcept;
        DescriptorPool& operator=(DescriptorPool&& other) noexcept;
        std::vector<VkDescriptorSet> allocateDescriptorSets(std::span<VkDescriptorSetLayout> layouts, uint32_t count);
        VkDescriptorSet allocateDescriptorSet(std::span<VkDescriptorSetLayout> layouts, uint32_t count);
    private:
        Device* m_device = nullptr;
        VkDescriptorPool m_pool = VK_NULL_HANDLE;
    };

    class Buffer;
    class Image;
    class Sampler;
    class DescriptorWriter
    {
    public:
        enum BufferType
        {
            UNIFORM,
            STORAGE
        };
        DescriptorWriter& writeBuffer(uint32_t binding, const Buffer& buffer, VkDeviceSize size, BufferType type);
        DescriptorWriter& writeImage(uint32_t binding, const Image& image, const Sampler& sampler);
        void update(Device& device, VkDescriptorSet set);
    private:
        struct BufferWrite
        {
            uint32_t binding;
            VkDescriptorBufferInfo info;
        };
        struct ImageWrite
        {
            uint32_t binding;
            VkDescriptorImageInfo info;
        };
        std::unordered_map<BufferType, BufferWrite> m_bufferwrites;
        std::vector<ImageWrite> m_imagewrites;
    };
} //namespace Lumos
