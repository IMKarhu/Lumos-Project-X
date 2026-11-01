#include "renderer.h"
#include <vulkan/vk_enum_string_helper.h>
#include <glm/gtc/matrix_transform.hpp>


namespace Lumos
{
    Renderer::Renderer()
    {
    }

    Renderer::~Renderer() {}

    void Renderer::update(float dt)
    {
    }

    //*******PRIVATE********//


    void Renderer::uniformBuffers()
    {
        // VkDeviceSize buffersize = sizeof(UniformBuffer);
        // m_uniformbuffers.reserve(m_maxframesinflight);
        // m_mappedubos.reserve(m_maxframesinflight);
        //
        // for (size_t i = 0; i < m_maxframesinflight; i++) {
        //     Buffer* buf = new Buffer(*m_device,
        //             buffersize,
        //             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        //             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //     m_mappedubos.push_back(buf->map());
        //     m_uniformbuffers.push_back(buf);
        // }
        // std::println("unformbuffer end");
    }

    void Renderer::updateUBOs(float dt, uint32_t currentimage)
    {
        // UniformBuffer ubo{};
        // auto proj = m_camera.projection();
        // // proj[1][1] *= -1;
        // ubo.projectionview = proj * m_camera.view();
        // std::memcpy(m_mappedubos[currentimage], &ubo, sizeof(ubo));
    }

} // namespace lumos
