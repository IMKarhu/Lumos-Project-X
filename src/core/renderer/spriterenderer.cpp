#include "spriterenderer.h"
#include "buffer.h"
#include "descriptor.h"
#include "resourcemanager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <print>
#include <cstring>

namespace Lumos
{
    SpriteRenderer::SpriteRenderer(Device& device, RendererConstructDescription rcd)
        : m_device(&device)
    {
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
        desc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        desc.polygonmode = VK_POLYGON_MODE_FILL;
        desc.cullmode = VK_CULL_MODE_NONE;
        desc.frontface = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        desc.dstalphablendfactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        m_pipeline = GraphicsPipeline::create(device, desc);
        vkDestroyShaderModule(device.device(), shadermodule, nullptr);
        m_image = std::make_unique<Image>(*m_device, m_width,
                            m_height,
                            VK_FORMAT_B8G8R8A8_SRGB,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        SamplerConfig config;
        config.enableanisotropy = false;
        config.filtermode = FilterMode::Nearest;
        m_sampler = std::make_unique<Sampler>(*m_device, config);
        // static_assert(offsetof(SpriteGPU, position) == 0);
        // static_assert(offsetof(SpriteGPU, size) == 8);
        // static_assert(offsetof(SpriteGPU, rotation) == 16);
        // static_assert(offsetof(SpriteGPU, uvrect) == 16);
        // static_assert(sizeof(SpriteGPU) == 32);
    }

    void SpriteRenderer::begin(Device::FrameInfo frameinfo, VkDescriptorSet globalset)
    {
        m_device->transitionImageLayout(m_image->image(),
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
        attachmentinfo.imageView = m_image->imageview();
        attachmentinfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentinfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentinfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentinfo.clearValue = clearvalue;

        VkRenderingInfoKHR renderinfo{};
        renderinfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderinfo.renderArea = {.offset = {0, 0}, .extent = {m_width, m_height}};
        renderinfo.layerCount = 1;
        renderinfo.colorAttachmentCount = 1;
        renderinfo.pColorAttachments = &attachmentinfo;

        vkCmdBeginRendering(frameinfo.cmdbuf, &renderinfo);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_width);
        viewport.height = static_cast<float>(m_height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;


        vkCmdSetViewport(frameinfo.cmdbuf, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = {m_width, m_height};

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

    void SpriteRenderer::submit(const Transform& t, uint32_t size, Device::FrameInfo frameinfo)
    {

        SpriteGPU s{};
        s.position = t.position;
        s.size = t.size;
        s.rotation = t.rotation;
        // s.uvrect = tile.uvrect;
        m_gpusprites.push_back(s);
    }

    void SpriteRenderer::end(Device::FrameInfo frameinfo, const ResourceManager& manager)
    {
        flush(frameinfo, manager);
        vkCmdEndRendering(frameinfo.cmdbuf);
        m_device->transitionImageLayout(m_image->image(),
                              frameinfo.cmdbuf,
                              VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                              VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                              VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    }

    void SpriteRenderer::map(Buffer& buffer)
    {
        if (m_gpusprites.empty()) {
            return;
        }
        const VkDeviceSize bytes = m_gpusprites.size() * sizeof(SpriteGPU);
        std::memcpy(buffer.map(), m_gpusprites.data(), bytes);
        buffer.unmap();
    }

    Image* SpriteRenderer::offscreenImage() const
    {
        return m_image.get();
    }

    Sampler* SpriteRenderer::offscreenSampler() const
    {
        return m_sampler.get();
    }

    //********PRIVATE*********//
   
    void SpriteRenderer::flush(Device::FrameInfo frameinfo, const ResourceManager& manager)
    {
        if (m_gpusprites.empty()) {
            return;
        }
        vkCmdBindDescriptorSets(frameinfo.cmdbuf,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipeline.layout(),
                1,
                1,
                &frameinfo.set,
                0,
                nullptr);
        PushConstant pushconstant;
        glm::mat4 transform = glm::mat4(1.0f);
        // transform = glm::translate(transform, glm::vec3(sprite.position, 0.0f));
        // transform = glm::rotate(transform, sprite.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        // transform = glm::scale(transform, glm::vec3(sprite.size, 0.0f));
        pushconstant.model = transform;
        vkCmdPushConstants(frameinfo.cmdbuf,
                m_pipeline.layout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(PushConstant),
                &pushconstant);

        vkCmdDraw(frameinfo.cmdbuf, static_cast<uint32_t>(m_gpusprites.size()) * 6, 1, 0, 0);
        m_gpusprites.clear();
    }
}
