#include "debugpass.h"


DebugPass::DebugPass(Lumos::Device& device)
    :m_device(&device)
{

}

DebugPass::~DebugPass(){}

void DebugPass::begin(Lumos::Device::FrameInfo frameinfo, VkDescriptorSet globalset)
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

void DebugPass::submit(Lumos::Device::FrameInfo frameinfo)
{

}

void DebugPass::end(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager)
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

void DebugPass::flush(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager)
{
        vkCmdPushConstants(frameinfo.cmdbuf,
                m_pipeline.layout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(PushConstant),
                &frameinfo.pushcst);
        VkBuffer buffers[] = { manager.getBufferByNameAndIndex("cubevertex", 0)->buffer() };
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(frameinfo.cmdbuf, 0, 1, buffers, offsets);
        auto ibuffer = manager.getBufferByNameAndIndex("cubeindex", 0);
        vkCmdBindIndexBuffer(frameinfo.cmdbuf, ibuffer->buffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frameinfo.cmdbuf, m_size, 1, 0, 0, 0);

}
