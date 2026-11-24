#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <expected>
#include <filesystem>

#include <glm/glm.hpp>
namespace Lumos
{
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline() = default;
        GraphicsPipeline(VkDevice device, VkPipeline pipeline, VkPipelineLayout layout);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline operator=(const GraphicsPipeline&) = delete;

        VkPipeline pipeline() const noexcept;
        VkPipelineLayout layout() const noexcept;
        void bind(VkCommandBuffer cmdbuf);

        template<typename T>
        void pushconstants(VkCommandBuffer cmdbuf, VkShaderStageFlags stages, const T& value) {
            vkCmdPushConstants(cmdbuf, m_layout, stages, 0, sizeof(T), &value);
        }
    private:
        VkDevice m_device = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_layout = VK_NULL_HANDLE;
    };
    std::vector<char> readFile(const std::string& filename);
    std::expected<std::vector<uint32_t>, std::string>
    readShaderFile(const std::filesystem::path& filepath);
} //namespace Lumos
