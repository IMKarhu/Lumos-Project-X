#include "rendersystem.h"
#include "windowing/window.h"
#include "renderer/device.h"
#include "resourcemanager.h"
// #include "renderer/3drenderer.h"
// #include "renderer/spriterenderer.h"
// #include "renderer/scenerenderer.h"
#include "renderer/buffer.h"
#include "ecs/ecs.h"
#include <glm/gtc/matrix_transform.hpp>

#include <cstring>

RenderSystem::RenderSystem(Lumos::Window& window, Lumos::VulkanConfig config)
{
    m_device = std::make_unique<Lumos::Device>(window, config);
    // m_factory = std::make_unique<Lumos::RenderFactory>();
}

RenderSystem::~RenderSystem() = default;

void RenderSystem::createRenderers(Lumos::ResourceManager& manager, Lumos::VulkanConfig config)
{
    manager.createTextureImage("offscreen", 1280, 720, VK_FORMAT_B8G8R8A8_SRGB);
    manager.createTextureWithoutSampler("depth", 1280, 720, m_device->depthformat(), true);
    for (size_t i = 0; i < config.framesinflight; i++) {
        Lumos::DescriptorWriter writer;
        auto& img = manager.getTextureByName("offscreen")->image();
        auto& sampler = manager.getTextureByName("offscreen")->sampler();
        writer.writeImage(0, img, sampler);
        writer.update(*m_device, manager.getSetByNameAndIndex("presentation", i));
    }

    PassConfig terrainconfig;
    terrainconfig.colorformat = m_device->defaultformat();
    const std::array terrainlayouts {
        manager.getLayoutByName("global")
    };
    terrainconfig.layouts = terrainlayouts;
    terrainconfig.shaderfilepath = "shaders/cube.spv";
    m_terrainpass = TerrainPass::create(*m_device, terrainconfig);
    PassConfig sceneconfig;
    sceneconfig.colorformat = m_device->defaultformat();
    const std::array scenelayouts {
        manager.getLayoutByName("presentation")
    };
    sceneconfig.layouts = scenelayouts; 
    sceneconfig.shaderfilepath = "shaders/presentation.spv";
    m_scenepass = ScenePass::create(*m_device, sceneconfig);
}

void RenderSystem::update(Ecs& ecs, Lumos::ResourceManager& manager, float dt)
{
    auto frameinfo = m_device->startFrame();
    for (auto ent : ecs.view<Lumos::Camera>()) {
        auto& c = ecs.getComponent<Lumos::CameraUBO>(ent);
        std::memcpy(manager.getBufferByNameAndIndex("global ubos", frameinfo.frameindex)->mappeddata(), &c, sizeof(c));
    }

    auto globalset = manager.getSetByNameAndIndex("global", frameinfo.frameindex);
    m_terrainpass.begin(frameinfo, manager);
    Lumos::Transform t;
    for (auto ent : ecs.view<Lumos::Size>()) {
        auto& s = ecs.getComponent<Lumos::Size>(ent);
        auto& p = ecs.getComponent<Lumos::Model>(ent);
        m_terrainpass.submit(s.size);
        frameinfo.pushcst.model = p.model;
    }
    m_terrainpass.end(frameinfo, manager);

    m_scenepass.begin(frameinfo);
    frameinfo.set = manager.getSetByNameAndIndex("presentation", frameinfo.frameindex);
    m_scenepass.end(frameinfo, manager);
    m_device->endFrame(frameinfo);
}

void RenderSystem::waitForDestroy()
{
    m_device->waitForDestroy();
}

Lumos::Device* RenderSystem::device() const
{
    return m_device.get();
}

glm::vec2 RenderSystem::snapToPixelGrid(glm::vec2 position)
{
    constexpr float pixelsPerWorldUnit = 32.0f;
    return glm::round(position * pixelsPerWorldUnit)
     / pixelsPerWorldUnit;
}
