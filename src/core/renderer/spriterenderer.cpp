#include "spriterenderer.h"
#include "errorcheck.h"
// #include "device.h"
#include "commandpool.h"
#include "buffer.h"
#include "descriptor.h"
#include "ktx.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <print>

namespace Lumos
{
    SpriteRenderer::SpriteRenderer(Device& device, RendererConstructDescription rcd)
        : m_device(&device)
    {
        // const std::array bindings {
        //     BindingConfig {
        //         .binding = 0,
        //         .type = DescriptorType::CombinedImageSampler,
        //         .count = 1,
        //         .flags = StageFlags::Fragment
        //     },
        //     BindingConfig {
        //         .binding = 1,
        //         .type = DescriptorType::Storage,
        //         .count = 1,
        //         .flags = StageFlags::Vertex
        //     }
        // };
        // m_desclayout = std::make_unique<DescriptorLayout>(device, bindings);

        auto code = readFile(rcd.shaderfilepath);
        VkShaderModule shadermodule = shaderModule(code, device.device());

        VkPipelineShaderStageCreateInfo vertexShaderStageCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shadermodule,
            .pName = "vertMain",
        };

        VkPipelineShaderStageCreateInfo fragmentShaderStageCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shadermodule,
            .pName = "fragMain"
        };
        GraphicsPipelineDescription desc;
        desc.shaderstages = {
            vertexShaderStageCI,
            fragmentShaderStageCI
        };
        // desc.vertexbindings.push_back(Vertex::bindingDescription());
        // desc.vertexattributes = Vertex::attributeDescription();
        desc.colorformat = rcd.colorformat;
        desc.descriptorlayouts.push_back(rcd.globlayout);
        desc.descriptorlayouts.push_back(rcd.layout);
        VkPushConstantRange pushconstant{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(PushConstant)
        };
        desc.pushconstantcount = 1;
        desc.pushconstantranges.push_back(pushconstant);
        m_pipeline = GraphicsPipeline::create(device, desc);
        vkDestroyShaderModule(device.device(), shadermodule, nullptr);

        //descriptorpools and sets
        // const std::array poolsizes {
        //     PoolConfig {
        //         .count = rcd.framesinflight,
        //         .type = DescriptorType::CombinedImageSampler
        //     },
        //     PoolConfig {
        //         .count = rcd.framesinflight,
        //         .type = DescriptorType::Storage
        //     },
        // };
        //
        // m_pool = std::make_unique<DescriptorPool>(device, poolsizes, rcd.framesinflight);
    }

    // SpriteRenderer::~SpriteRenderer()
    // {
    //
    // }

    void SpriteRenderer::begin(Device::FrameInfo frameinfo, VkDescriptorSet globalset)
    {
        m_device->transitionImageLayout(frameinfo.image,
                              frameinfo.cmdbuf,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                              VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                              {},
                              VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        VkClearValue clearvalue{};
        clearvalue.color = {0.0f, 0.0f, 0.0f, 1.0f};
        // clearvalue.depthStencil = {1.0f, 0};
        VkRenderingAttachmentInfo attachmentinfo{};
        attachmentinfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentinfo.imageView = frameinfo.imageview;
        attachmentinfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentinfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentinfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentinfo.clearValue = clearvalue;

        VkRenderingInfoKHR renderinfo{};
        renderinfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderinfo.renderArea = {.offset = {0, 0}, .extent = frameinfo.extent};
        renderinfo.layerCount = 1;
        renderinfo.colorAttachmentCount = 1;
        renderinfo.pColorAttachments = &attachmentinfo;

        vkCmdBeginRendering(frameinfo.cmdbuf, &renderinfo);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(frameinfo.extent.width);
        viewport.height = static_cast<float>(frameinfo.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;


        vkCmdSetViewport(frameinfo.cmdbuf, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = frameinfo.extent;

        vkCmdSetScissor(frameinfo.cmdbuf, 0, 1, &scissor);


        vkCmdBindPipeline(frameinfo.cmdbuf,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline.pipeline());

        vkCmdBindDescriptorSets(frameinfo.cmdbuf,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipeline.layout(),
                                0,
                                1,
                                &globalset,
                                0,
                                nullptr);
    }

    void SpriteRenderer::submit(const Transform& t, Device::FrameInfo frameinfo)
    {
        // if (m_currenttexture == nullptr) {
        //     m_currenttexture = sprite.texture;
        // }
        // if (sprite.texture != m_currenttexture) {
        //     flush(cmdbuf, frameindex);
        //     m_currenttexture = sprite.texture;
        // }
        if (m_gpusprites.size() >= MAXSPRITES) {
            // flush(cmdbuf, frameindex);
        }

        SpriteGPU s{};
        s.position = t.position;
        s.size = t.size;
        s.rotation = t.rotation;
        m_gpusprites.push_back(s);
    }

    void SpriteRenderer::end(Device::FrameInfo frameinfo)
    {
        vkCmdEndRendering(frameinfo.cmdbuf);
        m_device->transitionImageLayout(frameinfo.image,
                              frameinfo.cmdbuf,
                              VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                              VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                              VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                              VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                              {});

        flush(frameinfo, frameinfo.cmdbuf);
    }

    void SpriteRenderer::prepareSprites(std::vector<Sprite>& sprites, CommandPool& pool, RendererConstructDescription rcd)
    {
        createSpriteBuffer();
        createTexture("../textures/ground_dungeon7.ktx2", pool);
        std::array<VkDescriptorSetLayout, 2> layouts;
        layouts.fill(m_desclayout->layout());
        m_spriteset = m_pool->allocateDescriptorSets(layouts, rcd.framesinflight);

        for (size_t i = 0; i < rcd.framesinflight; i++) {
            DescriptorWriter writer;
            writer.writeImage(0, m_testtexture->image(), m_testtexture->sampler());
            writer.writeBuffer(1, *m_spritebuffer, sprites.size() * sizeof(SpriteGPU), DescriptorWriter::STORAGE);
            writer.update(*m_device, m_spriteset[i]);
        }
        for (auto& sprite : sprites) {
            sprite.texture = m_testtexture.get();
        }
    }

    //********PRIVATE*********//
   
    void SpriteRenderer::flush(Device::FrameInfo frameinfo, VkCommandBuffer cmdbuf)
    {
        if (m_gpusprites.empty()) {
            return;
        }
        uploadSprites();
        vkCmdBindDescriptorSets(cmdbuf,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipeline.layout(),
                1,
                1,
                &m_spriteset[frameinfo.frameindex],
                0,
                nullptr);
        PushConstant pushconstant;
        glm::mat4 transform = glm::mat4(1.0f);
        // transform = glm::translate(transform, glm::vec3(sprite.position, 0.0f));
        // transform = glm::rotate(transform, sprite.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        // transform = glm::scale(transform, glm::vec3(sprite.size, 0.0f));
        pushconstant.model = transform;
        vkCmdPushConstants(cmdbuf,
                m_pipeline.layout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(PushConstant),
                &pushconstant);

        vkCmdDraw(cmdbuf, static_cast<uint32_t>(m_gpusprites.size()) * 6, 1, 0, 0);
        m_gpusprites.clear();
    }

    void SpriteRenderer::createSpriteBuffer()
    {
        const VkDeviceSize bytes = MAXSPRITES * sizeof(SpriteGPU);
        m_spritebuffer = std::make_unique<Buffer>(*m_device, bytes,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    void SpriteRenderer::uploadSprites()
    {
        const VkDeviceSize bytes = m_gpusprites.size() * sizeof(SpriteGPU);
        std::memcpy(m_spritebuffer->map(), m_gpusprites.data(), bytes);
        m_spritebuffer->unmap();
    }

    void SpriteRenderer::createTexture(std::filesystem::path filepath, CommandPool& pool)
    {
        ktxTexture* texture = nullptr;
        const KTX_error_code result = ktxTexture_CreateFromNamedFile(filepath.string().c_str(),
                KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                &texture);

        if (result != KTX_SUCCESS) {
            std::println("failed to load ktx texture: {}", ktxErrorString(result));
            return;
        }
        std::println("texture size: {}x{}", texture->baseWidth, texture->baseHeight);
        uint32_t width = texture->baseWidth;
        uint32_t height = texture->baseHeight;
        ktx_size_t imagesize = ktxTexture_GetImageSize(texture, 0);
        ktx_uint8_t* texturedata = ktxTexture_GetData(texture);
        if (!texturedata) {
            std::println("data is null");
        }

        Buffer stagingbuffer(*m_device,
                imagesize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data = stagingbuffer.map();
        std::memcpy(data, texturedata, imagesize);
        stagingbuffer.unmap();

        Image image = Image(*m_device,
                            width,
                            height,
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        Sampler sampler = Sampler(*m_device);

        transitionImageLayout2(image.image(),
                               VK_FORMAT_R8G8B8A8_SRGB,
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               pool);
        copyBufferToImage(stagingbuffer, image.image(), width, height, pool);
        transitionImageLayout2(image.image(),
                               VK_FORMAT_R8G8B8A8_SRGB,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               pool);

        ktxTexture_Destroy(texture);
        m_testtexture = std::make_shared<Texture>(std::move(image), std::move(sampler));
    }

    void SpriteRenderer::copyBuffers(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size, CommandPool& pool)
    {
        VkCommandBuffer cmdbuf = beginSingleTimeCommands(pool);
        VkBufferCopy copyregion {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
        };
        vkCmdCopyBuffer(cmdbuf, srcBuffer.buffer(), dstBuffer.buffer(), 1, &copyregion);
        endSingleTimeCommands(cmdbuf, pool);
    }

    void SpriteRenderer::copyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height, CommandPool& pool)
    {
        VkCommandBuffer cmdbuf = beginSingleTimeCommands(pool);
        VkBufferImageCopy region {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {width, height, 1}
        };
        vkCmdCopyBufferToImage(cmdbuf,
                               buffer.buffer(),
                               image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);
        endSingleTimeCommands(cmdbuf, pool);
    }

    void SpriteRenderer::transitionImageLayout2(VkImage image,
                                                VkFormat format,
                                                VkImageLayout oldlayout,
                                                VkImageLayout newlayout,
                                                CommandPool& pool)
    {
        VkCommandBuffer cmdbuf = beginSingleTimeCommands(pool);
        VkImageMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = oldlayout,
            .newLayout = newlayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        VkPipelineStageFlags sourcestage;
        VkPipelineStageFlags destinationstage;
        if (oldlayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourcestage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationstage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newlayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourcestage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationstage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }
        vkCmdPipelineBarrier(cmdbuf,
                             sourcestage, destinationstage,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        endSingleTimeCommands(cmdbuf, pool);
    }


    VkCommandBuffer SpriteRenderer::beginSingleTimeCommands(CommandPool& pool)
    {
        VkCommandBufferAllocateInfo allocinfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool.pool(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        VkCommandBuffer cmdbuffer;
        VK_CHECK(vkAllocateCommandBuffers(m_device->device(), &allocinfo, &cmdbuffer));

        VkCommandBufferBeginInfo begininfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        VK_CHECK(vkBeginCommandBuffer(cmdbuffer, &begininfo));

        return cmdbuffer;
    }

    void SpriteRenderer::endSingleTimeCommands(VkCommandBuffer cmdbuf, CommandPool& pool)
    {
        vkEndCommandBuffer(cmdbuf);
        VkSubmitInfo submitinfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdbuf
        };
        VK_CHECK(vkQueueSubmit(m_device->presentqueue(), 1, &submitinfo, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(m_device->presentqueue()));
        vkFreeCommandBuffers(m_device->device(), pool.pool(), 1, &cmdbuf);
    }


}
