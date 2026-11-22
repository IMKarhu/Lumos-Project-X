#include "pipeline.h"
#include <fstream>

namespace Lumos
{
    GraphicsPipeline::GraphicsPipeline(VkDevice device, VkPipeline pipeline, VkPipelineLayout layout)
        : m_device(device)
        , m_pipeline(pipeline)
        , m_layout(layout)
    {
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_layout, nullptr);
    }

    VkPipelineLayout GraphicsPipeline::layout() const noexcept
    {
        return m_layout;
    }

    void GraphicsPipeline::bind(VkCommandBuffer cmdbuf)
    {
        vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open shader file");
        }
        size_t filesize = (size_t)file.tellg();
        std::vector<char> buffer(filesize);
        file.seekg(0);
        file.read(buffer.data(), filesize);
        file.close();
        return buffer;
    }
    std::expected<std::vector<uint32_t>, std::string>
    readShaderFile(const std::filesystem::path& filepath)
    {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            return std::unexpected("failed to open: "+ filepath.string());
        }
        
        const auto size = static_cast<size_t>(file.tellg());
        if (size == 0 || size % sizeof(uint32_t) != 0) {
            return std::unexpected(filepath.string() + ": size is not multiple of 4");
        }
        std::vector<uint32_t> buffer(size / sizeof(uint32_t));
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(size));
        constexpr uint32_t kSpirvMagic = 0x07230203; //some random magic number that AI gave me when I was debugging a bug
        if (buffer[0] != kSpirvMagic) {
            return std::unexpected(filepath.string() + ": not a SPIR-V file");
        }
        return buffer;
    }
} //namespace Lumos
