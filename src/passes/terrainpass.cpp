#include "terrainpass.h"
#include "renderer/vertex.h"
#include "renderer/buffer.h"
#include <utility>

TerrainPass::~TerrainPass()
{
    destroy();
}

TerrainPass::TerrainPass(TerrainPass&& other) noexcept
    : m_device(std::exchange(other.m_device, nullptr))
    , m_pipeline(std::exchange(other.m_pipeline, {}))
{

}

TerrainPass& TerrainPass::operator=(TerrainPass&& other) noexcept
{
    if (&other == this) {
        return *this;
    }

    destroy();
    m_device = std::exchange(other.m_device, nullptr);
    m_pipeline = std::exchange(other.m_pipeline, {});
    return *this;
}



void TerrainPass::submit(uint32_t size)
{
    m_size = size;
}

void TerrainPass::begin(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager)
{
    auto tex = manager.getTextureByName("offscreen");
    auto depth = manager.getTextureByName("depth");
    m_device->transitionImageLayout(tex->image().image(),
            frameinfo.cmdbuf,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            {},
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);
    
    m_device->transitionImageLayout(depth->image().image(),
            frameinfo.cmdbuf,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT);
    
    std::array<VkClearValue, 2> clearvalues{};
    clearvalues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearvalues[1].depthStencil = {1.0f, 0};
    
    VkRenderingAttachmentInfo attachmentinfo{};
    attachmentinfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    attachmentinfo.imageView = tex->image().imageview();
    attachmentinfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentinfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentinfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentinfo.clearValue = clearvalues[0];
    
    VkRenderingAttachmentInfo depthinfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = depth->image().imageview(),
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = clearvalues[1]
    };
        
    
    VkRenderingInfoKHR renderinfo{};
    renderinfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderinfo.renderArea = {.offset = {0, 0}, .extent = {m_width, m_height}};
    renderinfo.layerCount = 1;
    renderinfo.colorAttachmentCount = 1;
    renderinfo.pColorAttachments = &attachmentinfo;
    renderinfo.pDepthAttachment = &depthinfo;
    
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
    
    auto set = manager.getSetByNameAndIndex("global", frameinfo.frameindex);
    vkCmdBindDescriptorSets(frameinfo.cmdbuf,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline.layout(),
                            0,
                            1,
                            &set,
                            0,
                            nullptr);

}

void TerrainPass::end(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager)
{
    auto tex = manager.getTextureByName("offscreen");
    flush(frameinfo, manager);
    vkCmdEndRendering(frameinfo.cmdbuf);
    m_device->transitionImageLayout(tex->image().image(),
            frameinfo.cmdbuf,
            VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);
}

TerrainPass TerrainPass::create(Lumos::Device& device, PassConfig config)
{
    TerrainPass pass;
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
    desc.vertexbindings.push_back(Lumos::Vertex::bindingDescription());
    desc.vertexattributes = Lumos::Vertex::attributeDescription();
    desc.colorformat = config.colorformat;
    desc.descriptorlayouts = config.layouts;
    VkPushConstantRange pushconstant{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(PushConstant)
    };
    desc.pushconstantcount = 1;
    desc.pushconstantranges.push_back(pushconstant);
    desc.depthtest = true;
    desc.depthwrite = true;
    desc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    desc.polygonmode = VK_POLYGON_MODE_FILL;
    desc.cullmode = VK_CULL_MODE_BACK_BIT;
    desc.frontface = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    desc.dstalphablendfactor = VK_BLEND_FACTOR_ZERO;
    desc.depthformat = device.depthformat();

    pass.m_pipeline = Lumos::GraphicsPipeline::create(device, desc);
    vkDestroyShaderModule(device.device(), shadermodule, nullptr);
    return pass;
}

void TerrainPass::flush(Lumos::Device::FrameInfo frameinfo, const Lumos::ResourceManager& manager)
{
        // vkCmdBindDescriptorSets(frameinfo.cmdbuf,
        //         VK_PIPELINE_BIND_POINT_GRAPHICS,
        //         m_pipeline.layout(),
        //         1,
        //         1,
        //         &frameinfo.set,
        //         0,
        //         nullptr);
        // glm::mat4 transform = glm::mat4(1.0f);
        // transform = glm::translate(transform, glm::vec3(sprite.position, 0.0f));
        // transform = glm::rotate(transform, sprite.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        // transform = glm::scale(transform, glm::vec3(sprite.size, 0.0f));
        // pushconstant.model = transform;
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
        // vkCmdDraw(cmdbuf, static_cast<uint32_t>(m_gpusprites.size()) * 6, 1, 0, 0);

}

void TerrainPass::destroy()
{
    m_device = nullptr;
}
