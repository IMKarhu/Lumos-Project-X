#pragma once
#include "renderer/device.h"
#include "resourcemanager.h"
#include "renderer/pipeline.h"
#include "passconfig.h"

class ScenePass
{
public:
    ScenePass() = default;
    ~ScenePass();

    ScenePass(const ScenePass&) = delete;
    ScenePass operator=(const ScenePass&) = delete;

    ScenePass(ScenePass&& other) noexcept;
    ScenePass& operator=(ScenePass&& other) noexcept;

    void begin(Lumos::Device::FrameInfo frameinfo);
    void end(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager);
    static ScenePass create(Lumos::Device& device, PassConfig config);
private:
    Lumos::Device* m_device = nullptr;
    Lumos::GraphicsPipeline m_pipeline;
    void flush(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager);
    void destroy();
};
