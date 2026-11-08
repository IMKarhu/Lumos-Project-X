#pragma once
#include "renderer/device.h"
#include "renderer/pipeline.h"

namespace Lumos
{
    class ResourceManager;
}

class DebugPass
{
public:
    DebugPass(Lumos::Device& device);
    ~DebugPass();
    void begin(Lumos::Device::FrameInfo frameinfo, VkDescriptorSet globalset);
    void submit(Lumos::Device::FrameInfo frameinfo);
    void end(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager);
private:
    Lumos::Device* m_device = nullptr;
    Lumos::GraphicsPipeline m_pipeline;
    void flush(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager);
};
