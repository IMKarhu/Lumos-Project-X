#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <span>

#include <glm/glm.hpp>
namespace Lumos
{

    /*
     * Forward Declarations
     */
    class Device;

    struct PipelineConfig
    {
        std::string shaderfile;
        std::span<const VkDescriptorSetLayout> layouts;
        VkFormat colorformat;
        VkPrimitiveTopology topology;
    };

    class GraphicsPipeline
    {
    public:
        explicit GraphicsPipeline(Device& device, const PipelineConfig& config);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline operator=(const GraphicsPipeline&) = delete;

        [[nodiscard]] VkPipeline pipeline() const noexcept;
        VkPipelineLayout layout() const noexcept;
        void createPipeline(const std::string& shaderpath);
        void bind(VkCommandBuffer cmdbuf);
    private:
        void createPipelineLayout();
        VkShaderModule shaderModule(const std::vector<char>& buffer, VkDevice device);
        Device& m_device;
        PipelineConfig m_config;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_layout = VK_NULL_HANDLE;
    };
    std::vector<char> readFile(const std::string& filename);
} //namespace Lumos
