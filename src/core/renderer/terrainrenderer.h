#pragma once
#include "baserenderer.h"
#include "device.h"
#include "pipeline.h"

#ifdef _WIN32
    #ifdef CORE_EXPORT
    #define CORE_API __declspec(dllexport)
    #else
    #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API
#endif


namespace Lumos
{
    struct RendererConstructDescription
    {
        uint32_t framesinflight = 2;
        VkFormat colorformat;
        VkDescriptorSetLayout globlayout;
        VkDescriptorSetLayout layout;
        std::string shaderfilepath;
    };

    class CORE_API TerrainRenderer : public RendererBase
    {
    public:
        TerrainRenderer(Device& device, RendererConstructDescription rcd);
        ~TerrainRenderer() override = default;

        void begin(Device::FrameInfo frameinfo, VkDescriptorSet globalset) override;
        void submit(const Transform& t, Device::FrameInfo frameinfo) override;
        void end(Device::FrameInfo frameinfo) override;
        void map(Buffer& buffer) override;

    private:
        Device* m_device = nullptr;
        GraphicsPipeline m_pipeline;
    };
} //namespace Lumos
