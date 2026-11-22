#pragma once
#include <vulkan/vulkan.h>
#include <span>
#include <cstdint>
#include <expected>
#include <vector>
#include <memory>

#include "pipeline.h"

namespace Lumos
{
    class Device;
    class PipelineBuilder
    {
    public:
        auto& shader(std::span<const uint32_t> binary)
        {
            m_shader = binary;
            return *this;
        }

        auto& vertexInput(std::span<const VkVertexInputBindingDescription> bindings,
                          std::span<const VkVertexInputAttributeDescription> attributes)
        {
            m_bindings.assign(bindings.begin(), bindings.end());
            m_attributes.assign(attributes.begin(), attributes.end());
            return *this;
        }

        auto& topology(VkPrimitiveTopology topology)
        {
            m_topology = topology;
            return *this;
        }

        auto& cullMode(VkCullModeFlags mode, VkFrontFace front = VK_FRONT_FACE_COUNTER_CLOCKWISE)
        {
            m_cull = mode;
            m_frontface = front;
            return *this;
        }

        auto& depth(bool test, bool write, VkCompareOp op = VK_COMPARE_OP_LESS)
        {
            m_depthtest = test;
            m_depthwrite = write;
            m_depthop = op;
            return *this;
        }

        auto& deptBias(bool enable = true)
        {
            m_depthbias = enable;
            return *this;
        }

        auto& alphablend(bool enable = true)
        {
            m_blend = enable;
            return *this;
        }

        auto& colorFormats(std::initializer_list<VkFormat> formats)
        {
            m_colorformats.assign(formats.begin(), formats.end());
            return *this;
        }

        auto& depthFormat(VkFormat format)
        {
            m_depthformat = format;
            return *this;
        }

        auto& descriptorSetLayouts(std::span<const VkDescriptorSetLayout> layouts)
        {
            m_setlayouts.assign(layouts.begin(), layouts.end());
            return *this;
        }

        auto& pushConstantRange(VkShaderStageFlags stages, uint32_t size, uint32_t offset)
        {
            m_pushranges.push_back({stages, size, offset});
            return *this;
        }

        std::expected<std::unique_ptr<GraphicsPipeline>, VkResult>
        build(Device& device, VkPipelineCache cache = VK_NULL_HANDLE) const;
        
    private:
        std::span<const uint32_t> m_shader;
        std::vector<VkVertexInputBindingDescription> m_bindings;
        std::vector<VkVertexInputAttributeDescription> m_attributes;

        VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkCullModeFlags m_cull = VK_CULL_MODE_BACK_BIT;
        VkFrontFace m_frontface = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        bool m_depthtest = true;
        bool m_depthbias = true;
        bool m_depthwrite = true;
        bool m_blend = true;
        VkCompareOp m_depthop = VK_COMPARE_OP_LESS;

        std::vector<VkFormat> m_colorformats;
        VkFormat m_depthformat;
        std::vector<VkDescriptorSetLayout> m_setlayouts;
        std::vector<VkPushConstantRange> m_pushranges;
    };
} // namespace Lumos
