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

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(Device& device,
                VkDescriptorSetLayout camerasetlayout,
                VkDescriptorSetLayout entitysetlayout,
                VkDescriptorSetLayout texturesetlayout, 
                VkFormat colorformat = VK_FORMAT_B8G8R8A8_SRGB);
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
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_camerasetlayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_entitysetlayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_texturesetlayout = VK_NULL_HANDLE;

        VkFormat m_colorformat = VK_FORMAT_B8G8R8A8_SRGB;



    };
    std::vector<char> readFile(const std::string& filename);
} //namespace Lumos
