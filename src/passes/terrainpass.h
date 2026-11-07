#pragma once
#include "renderer/device.h"
#include "resourcemanager.h"
#include "renderer/pipeline.h"
#include "passconfig.h"

class TerrainPass
{
public:
    TerrainPass() = default;
    ~TerrainPass();

    TerrainPass(const TerrainPass&) = delete;
    TerrainPass operator=(const TerrainPass&) = delete;

    TerrainPass(TerrainPass&& other) noexcept;
    TerrainPass& operator=(TerrainPass&& other) noexcept;

    void submit(uint32_t size, glm::mat4 model);
    void begin(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager);
    void end(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager);
    static TerrainPass create(Lumos::Device& device, PassConfig config);
private:
    Lumos::Device* m_device = nullptr;
    Lumos::GraphicsPipeline m_pipeline;
    uint32_t m_width = 1280;
    uint32_t m_height = 720;
    std::vector<uint32_t> m_sizes;
    std::vector<glm::mat4> m_pushconstantmodels;
    uint32_t m_size = 0;
    struct PushConstant
    {
        glm::mat4 model;
    };
    void flush(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager);
    void destroy();
};
