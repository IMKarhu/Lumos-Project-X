#include "descriptor.h"
#include "device.h"
#include "errorcheck.h"
#include "buffer.h"
#include "image.h"
#include "sampler.h"
#include <array>
#include <cassert>
#include <utility>
#include <print>

namespace Lumos
{
    static VkDescriptorType typeHelper(DescriptorType type)
    {
        switch(type)
        {
        case Lumos::DescriptorType::Uniform:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case Lumos::DescriptorType::Storage:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            break;
        case Lumos::DescriptorType::CombinedImageSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        }
    }

    static VkShaderStageFlags shaderStageHelper(StageFlags flags)
    {
        switch(flags)
        {
        case Lumos::StageFlags::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case Lumos::StageFlags::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        default:
            return VK_SHADER_STAGE_VERTEX_BIT;
        }
    }

    DescriptorLayout::DescriptorLayout(Device& device, std::span<const BindingConfig> bindings)
        : m_device(&device)
    {
        std::vector<VkDescriptorSetLayoutBinding> binds;
        for (auto& config : bindings) {
            VkDescriptorSetLayoutBinding b{};
            b.binding = config.binding;
            b.descriptorCount = config.count;
            b.descriptorType = typeHelper(config.type);
            b.stageFlags = shaderStageHelper(config.flags);
            binds.push_back(b);
        }
        VkDescriptorSetLayoutCreateInfo layoutinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(binds.size()),
            .pBindings = binds.data()
        };
        VK_CHECK(vkCreateDescriptorSetLayout(m_device->device(), &layoutinfo, nullptr, &m_layout));
    }

    DescriptorLayout::~DescriptorLayout()
    {

    }

    DescriptorLayout::DescriptorLayout(DescriptorLayout&& other) noexcept
        : m_device(other.m_device)
        , m_layout(std::exchange(other.m_layout, VK_NULL_HANDLE))
    {

    }
    
    DescriptorLayout& DescriptorLayout::operator=(DescriptorLayout&& other) noexcept
    {
        if (&other == this) {
            return *this;
        }
        if (m_layout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_device->device(), m_layout, nullptr);
        }
        m_device = std::exchange(other.m_device, nullptr);
        m_layout = std::exchange(other.m_layout, VK_NULL_HANDLE);
        return *this;
    }

    VkDescriptorSetLayout DescriptorLayout::layout() const noexcept
    {
        return m_layout;
    }

    // ***********DESCRIPTORPOOL***********
    
    DescriptorPool::DescriptorPool(Device& device, std::span<const PoolConfig> poolconfig, uint32_t maxsets)
        : m_device(&device)
    {
        std::vector<VkDescriptorPoolSize> pools;
        for (auto& config : poolconfig) {
            std::println("config type: {}", (int)typeHelper(config.type));
            VkDescriptorPoolSize ps{};
            ps.descriptorCount = config.count;
            ps.type = typeHelper(config.type);
            std::println("config type: {}", (int)ps.type);

            pools.push_back(ps);
        }

        VkDescriptorPoolCreateInfo poolinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = maxsets,
            .poolSizeCount = static_cast<uint32_t>(pools.size()),
            .pPoolSizes = pools.data()
        };

        VK_CHECK(vkCreateDescriptorPool(m_device->device(), &poolinfo, nullptr, &m_pool));
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(m_device->device(), m_pool, nullptr);
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
    {
        assert(0 && "TODO IMPLEMENT MOVE CONSTRUCTOR");
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept
    {
        assert(0 && "TODO IMPLEMENT MOVE ASSIGNMENT");
        return *this;
    }

    std::vector<VkDescriptorSet> DescriptorPool::allocateDescriptorSets(std::span<VkDescriptorSetLayout> layouts, uint32_t count)
    {
        VkDescriptorSetAllocateInfo allocinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_pool,
            .descriptorSetCount = count,
            .pSetLayouts = layouts.data()
        };
        std::vector<VkDescriptorSet> sets;
        sets.resize(count);
        VK_CHECK(vkAllocateDescriptorSets(m_device->device(), &allocinfo, sets.data()));
        return sets;
    }

    VkDescriptorSet DescriptorPool::allocateDescriptorSet(std::span<VkDescriptorSetLayout> layouts, uint32_t count)
    {
        if (layouts.empty()) {
            std::println("empty");
        }
        VkDescriptorSetAllocateInfo allocinfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_pool,
            .descriptorSetCount = count,
            .pSetLayouts = layouts.data()
        };
        VkDescriptorSet set;
        VK_CHECK(vkAllocateDescriptorSets(m_device->device(), &allocinfo, &set));
        return nullptr;
    }

    // ***********DESCRIPTOR SET WRITER***********

    DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, const Buffer& buffer, VkDeviceSize size, BufferType type)
    {
        m_bufferwrites[type] =  {
                .binding = binding,
                .info = {
                    .buffer = buffer.buffer(),
                    .offset = 0,
                    .range = size
                }
        };
        return *this;
    }

    DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, const Image& image, const Sampler& sampler)
    {
        m_imagewrites.push_back({
                .binding = binding,
                .info = {
                    .sampler = sampler.sampler(),
                    .imageView = image.imageview(),
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                }
        });
        return *this;
    }

    void DescriptorWriter::update(Device& device, VkDescriptorSet set)
    {
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(m_bufferwrites.size() + m_imagewrites.size());
        for (const auto& [key, value] : m_bufferwrites) {
            if (key == BufferType::UNIFORM) {
                writes.push_back({
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = set,
                        .dstBinding = value.binding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pBufferInfo = &value.info
                    });
            }
            if (key == BufferType::STORAGE) {
                writes.push_back({
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = set,
                        .dstBinding = value.binding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pBufferInfo = &value.info
                    });
            }
        }

        for (const auto& imgwrite : m_imagewrites) {
            writes.push_back({
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set,
                    .dstBinding = imgwrite.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = &imgwrite.info
            });
        }
        vkUpdateDescriptorSets(device.device(),
                               static_cast<uint32_t>(writes.size()),
                               writes.data(),
                               0,
                               nullptr);
    }

} // namespace Lumos
