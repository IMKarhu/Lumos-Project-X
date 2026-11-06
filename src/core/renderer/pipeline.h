#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <span>

namespace Lumos
{
    struct GraphicsPipelineDescription {
        std::vector<VkPipelineShaderStageCreateInfo> shaderstages;
        std::vector<VkVertexInputBindingDescription> vertexbindings;
        std::vector<VkVertexInputAttributeDescription> vertexattributes;
        std::span<const VkDescriptorSetLayout> descriptorlayouts;
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPolygonMode polygonmode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullmode = VK_CULL_MODE_NONE;//OR FOR NOW VK_CULL_MODE_NONE VK_CULL_MODE_BACK_BIT
        VkFrontFace frontface = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        bool depthtest = true;
        bool depthwrite = true;
        bool dynamicviewport = true;
        bool dynamicscissor = true;
        VkFormat colorformat = VK_FORMAT_UNDEFINED;
        VkFormat depthformat = VK_FORMAT_UNDEFINED;
        VkBlendFactor dstalphablendfactor = VK_BLEND_FACTOR_ZERO; 
        uint32_t pushconstantcount = 0;
        std::vector<VkPushConstantRange> pushconstantranges;
    };

    class Device;

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline() = default;
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline operator=(const GraphicsPipeline&) = delete;

        GraphicsPipeline(GraphicsPipeline&& other) noexcept;
        GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

        [[nodiscard]] VkPipeline pipeline() const noexcept;
        [[nodiscard]] VkPipelineLayout layout() const noexcept;

        explicit operator bool() const
        {
            return m_pipeline != VK_NULL_HANDLE;
        }

        static GraphicsPipeline create(Device& device, const GraphicsPipelineDescription& desc);

    private:
        Device* m_device = nullptr;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_layout = VK_NULL_HANDLE;
        void destroy();
    };

    std::vector<char> readFile(const std::string& filename);
    VkShaderModule shaderModule(const std::vector<char>& buffer, VkDevice device);

} //namespace Lumos
