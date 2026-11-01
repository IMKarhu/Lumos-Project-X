#pragma once
#include "baserenderer.h"
#include "device.h"
#include "pipeline.h"
#include "descriptor.h"
#include "buffer.h"
#include "sprite.h"
#include "texture.h"
#include "../ecs/components.h"
#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <filesystem>

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
    // class Device;
    class CommandPool;
    class Camera2D;
    class CORE_API SpriteRenderer : public RendererBase
    {
    public:
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
        void prepareSprites(std::vector<Sprite>& sprites, CommandPool& pool, RendererConstructDescription rcd);
    private:
        constexpr static std::size_t MAXSPRITES = 1000;
        Device* m_device = nullptr;
        GraphicsPipeline m_pipeline;
        
        std::unique_ptr<DescriptorLayout> m_desclayout;
        std::unique_ptr<DescriptorPool> m_pool;
        std::vector<VkDescriptorSet> m_spriteset;

        std::unique_ptr<Buffer> m_spritebuffer;
        void* mappedbuffer;
        std::shared_ptr<Texture> m_testtexture;

        struct PushConstant
        {
            glm::mat4 model;
        };
        struct SpriteGPU
        {
            alignas(8)glm::vec2 position;
            alignas(8)glm::vec2 size;
            float rotation;
        };

        std::vector<SpriteGPU> m_gpusprites;
        Texture* m_currenttexture = nullptr;
        void flush(Device::FrameInfo frameinfo, VkCommandBuffer cmdbuf);
        void createSpriteBuffer();
        void uploadSprites();
        void createTexture(std::filesystem::path filepath, CommandPool& pool);
        void copyBuffers(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size, CommandPool& pool);
        void copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height, CommandPool& pool);
        void transitionImageLayout2(VkImage image,
                                    VkFormat format,
                                    VkImageLayout oldlayout,
                                    VkImageLayout newlayout,
                                    CommandPool& pool);
        VkCommandBuffer beginSingleTimeCommands(CommandPool& pool);
        void endSingleTimeCommands(VkCommandBuffer cmdbuf, CommandPool& pool);

    };
} //namespace Lumos
