#pragma once
#include "baserenderer.h"
#include "device.h"
#include "pipeline.h"
#include "image.h"
#include "../ecs/components.h"
#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

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
    class Buffer;
    class CORE_API SpriteRenderer : public RendererBase
    {
    public:
        struct SpriteGPU
        {
            alignas(8)glm::vec2 position;
            alignas(8)glm::vec2 size;
            float rotation;
        };

        //Horrible name for a struct, but didnt come up with a better one
        struct RendererConstructDescription
        {
            uint32_t framesinflight = 2;
            VkFormat colorformat;
            VkDescriptorSetLayout globlayout;
            VkDescriptorSetLayout layout;
            std::string shaderfilepath;
        };
        SpriteRenderer(Device& device, RendererConstructDescription rcd);
        ~SpriteRenderer() override = default;

        void begin(Device::FrameInfo frameinfo, VkDescriptorSet globalset) override;
        void submit(const Transform& t, Device::FrameInfo frameinfo) override;
        void end(Device::FrameInfo frameinfo) override;
        void map(Buffer& buffer) override;
    private:
        Device* m_device = nullptr;
        GraphicsPipeline m_pipeline;
        std::vector<SpriteGPU> m_gpusprites;
        std::unique_ptr<Image> m_image;
        
        struct PushConstant
        {
            glm::mat4 model;
        };

        void flush(Device::FrameInfo frameinfo, VkCommandBuffer cmdbuf);
    };
} //namespace Lumos
