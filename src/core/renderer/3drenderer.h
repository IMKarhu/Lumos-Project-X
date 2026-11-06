#pragma once
#include "baserenderer.h"
#include "device.h"
#include "pipeline.h"
#include "sampler.h"
#include "image.h"

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
    class Image;
    class Sampler;
    class Buffer;
    class CORE_API DRenderer : public RendererBase
    {
    public:
        //Horrible name for a struct, but didnt come up with a better one
        struct RendererConstructDescription
        {
            uint32_t framesinflight = 2;
            VkFormat colorformat;
            std::span<const VkDescriptorSetLayout> layouts;
            VkDescriptorSetLayout globlayout;
            VkDescriptorSetLayout layout;
            std::string shaderfilepath;
        };
        DRenderer(Device& device, RendererConstructDescription rcd);
        ~DRenderer() override = default;

        void begin(Device::FrameInfo frameinfo, VkDescriptorSet globalset) override;
        void submit(const Transform& t, uint32_t size, Device::FrameInfo frameinfo) override;
        void end(Device::FrameInfo frameinfo, const ResourceManager& manager) override;
        void map(Buffer& buffer) override;
        Image* offscreenImage() const override;
        Sampler* offscreenSampler() const override;
    private:
        Device* m_device = nullptr;
        GraphicsPipeline m_pipeline;
        std::unique_ptr<Image> m_image;
        std::unique_ptr<Image> m_depthimage;
        std::unique_ptr<Sampler> m_sampler;
        uint32_t m_width = 1280;
        uint32_t m_height = 720;
        uint32_t m_size = 0;
        
        struct PushConstant
        {
            glm::mat4 model;
        };

        void flush(Device::FrameInfo frameinfo, const ResourceManager& manager);

    };
} //namespace Lumos
