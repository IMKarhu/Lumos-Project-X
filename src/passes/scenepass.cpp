#include "scenepass.h"
#include "renderer/device.h"
#include <utility>

ScenePass::~ScenePass() {}

ScenePass::ScenePass(ScenePass&& other) noexcept
    : m_device(std::exchange(other.m_device, nullptr))
    , m_pipeline(std::exchange(other.m_pipeline, {}))
{

}

ScenePass& ScenePass::operator=(ScenePass&& other) noexcept
{
    if (&other == this) {
        return *this;
    }

    destroy();
    m_device = std::exchange(other.m_device, nullptr);
    m_pipeline = std::exchange(other.m_pipeline, {});
    return *this;
}


void ScenePass::begin(Lumos::Device::FrameInfo frameinfo)
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

void ScenePass::end(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager)
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

ScenePass ScenePass::create(Lumos::Device& device, PassConfig config)
{
    ScenePass pass;
    pass.m_device = &device;
    auto code = Lumos::readFile(config.shaderfilepath);
    VkShaderModule shadermodule = Lumos::shaderModule(code, device.device());

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
    Lumos::GraphicsPipelineDescription desc;
    desc.shaderstages = {
        vertexShaderStageCI,
        fragmentShaderStageCI
    };
    desc.colorformat = config.colorformat;
    desc.descriptorlayouts = config.layouts;
    desc.pushconstantcount = 0;
    desc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    desc.polygonmode = VK_POLYGON_MODE_FILL;
    desc.cullmode = VK_CULL_MODE_BACK_BIT;
    desc.frontface = VK_FRONT_FACE_CLOCKWISE;
    desc.dstalphablendfactor = VK_BLEND_FACTOR_ZERO;
    pass.m_pipeline = Lumos::GraphicsPipeline::create(device, desc);
    vkDestroyShaderModule(device.device(), shadermodule, nullptr);

    return pass;
}

void ScenePass::flush(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager)
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

void ScenePass::destroy()
{
    m_device = nullptr;
}
