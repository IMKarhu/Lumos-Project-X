#pragma once
#include "baserenderer.h"
#include "device.h"
#include "pipeline.h"
#include "image.h"
#include "sampler.h"

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


    class CORE_API SceneRenderer : public RendererBase
    {
    public:
        struct RendererConstructDescription
        {
            uint32_t framesinflight = 2;
            VkFormat colorformat;
            VkDescriptorSetLayout globlayout;
            VkDescriptorSetLayout layout;
            std::string shaderfilepath;
        };
        SceneRenderer(Device& device, RendererConstructDescription rcd);
        ~SceneRenderer() override = default;

        void begin(Device::FrameInfo frameinfo, VkDescriptorSet globalset) override;
        void submit(const Transform& t, const Tile& tile, Device::FrameInfo frameinfo) override;
        void end(Device::FrameInfo frameinfo) override;
        void map(Buffer& buffer) override;

        //not used in presentation
        Image* offscreenImage() const override;
        Sampler* offscreenSampler() const override;

    private:
        Device* m_device = nullptr;
        GraphicsPipeline m_pipeline;
        void flush(Device::FrameInfo frameinfo);
    };
} //namespace Lumos
