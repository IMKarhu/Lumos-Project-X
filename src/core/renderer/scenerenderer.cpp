#include "scenerenderer.h"
#include "resourcemanager.h"


namespace Lumos
{
    SceneRenderer::SceneRenderer(Device& device, RendererConstructDescription rcd)
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
        desc.colorformat = rcd.colorformat;
        desc.descriptorlayouts.push_back(rcd.layout);
        desc.pushconstantcount = 0;
        desc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        desc.polygonmode = VK_POLYGON_MODE_FILL;
        desc.cullmode = VK_CULL_MODE_BACK_BIT;
        desc.frontface = VK_FRONT_FACE_CLOCKWISE;
        desc.dstalphablendfactor = VK_BLEND_FACTOR_ZERO;
        m_pipeline = GraphicsPipeline::create(device, desc);
        vkDestroyShaderModule(device.device(), shadermodule, nullptr);
    }

    void SceneRenderer::begin(Device::FrameInfo frameinfo, VkDescriptorSet globalset)
    {
        m_device->transitionImageLayout(frameinfo.image,
                              frameinfo.cmdbuf,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                              VK_PIPELINE_STAGE_2_NONE,
                              VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VK_ACCESS_2_NONE,
                              VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT);

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
    }

    /*
     * we don't actually need to submit anything in scene rendering
    */
    void SceneRenderer::submit(const Transform& t, uint32_t size, Device::FrameInfo frameinfo) {}

    void SceneRenderer::end(Device::FrameInfo frameinfo, const ResourceManager& manager)
    {
        flush(frameinfo, manager);
        vkCmdEndRendering(frameinfo.cmdbuf);
        m_device->transitionImageLayout(frameinfo.image,
                              frameinfo.cmdbuf,
                              VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                              VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                              VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                              VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                              VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                              {},
                              VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void SceneRenderer::map(Buffer& buffer){}
    Image* SceneRenderer::offscreenImage() const {return nullptr;}
    Sampler* SceneRenderer::offscreenSampler() const {return nullptr;}



    /**********PRIVATE**********/
    

    void SceneRenderer::flush(Device::FrameInfo frameinfo, const ResourceManager& manager)
    {
        vkCmdBindDescriptorSets(frameinfo.cmdbuf,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipeline.layout(),
                0,
                1,
                &frameinfo.set,
                0,
                nullptr);
        vkCmdDraw(frameinfo.cmdbuf, 3, 1, 0, 0);
    }

}
