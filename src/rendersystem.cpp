#include "rendersystem.h"
#include "windowing/window.h"
#include "renderer/device.h"
#include "resourcemanager.h"
#include "renderer/3drenderer.h"
#include "renderer/spriterenderer.h"
#include "renderer/scenerenderer.h"
#include "renderer/buffer.h"
#include "ecs/ecs.h"
#include <glm/gtc/matrix_transform.hpp>

#include <cstring>

RenderSystem::RenderSystem(Lumos::Window& window, Lumos::VulkanConfig config)
{
    m_device = std::make_unique<Lumos::Device>(window, config);
    m_factory = std::make_unique<Lumos::RenderFactory>();
}

RenderSystem::~RenderSystem() = default;

void RenderSystem::createRenderers(Lumos::ResourceManager& manager, Lumos::VulkanConfig config)
{
    // Lumos::SpriteRenderer::RendererConstructDescription rcd;
    // rcd.shaderfilepath = "shaders/sprite.spv";
    // rcd.colorformat = m_device->defaultformat();
    // rcd.framesinflight = config.framesinflight;
    // rcd.globlayout = manager.getLayoutByName("global");
    // rcd.layout = manager.getLayoutByName("sprite");
    // m_spriterenderer = m_factory->createSpriteRenderer(*m_device, rcd);
    Lumos::DRenderer::RendererConstructDescription cubeconfig;
    cubeconfig.colorformat = m_device->defaultformat();
    cubeconfig.framesinflight = config.framesinflight;
    cubeconfig.globlayout = manager.getLayoutByName("global");
    // cubeconfig.layout = manager.getLayoutByName("cube");
    cubeconfig.shaderfilepath = "shaders/cube.spv";
    m_cuberenderer = std::make_unique<Lumos::DRenderer>(*m_device, cubeconfig);

    for (size_t i = 0; i < config.framesinflight; i++) {
        Lumos::DescriptorWriter writer;
        auto img = m_cuberenderer->offscreenImage();
        auto sampler = m_cuberenderer->offscreenSampler();
        writer.writeImage(0, *img, *sampler);
        writer.update(*m_device, manager.getSetByNameAndIndex("presentation", i));
    }

    Lumos::SceneRenderer::RendererConstructDescription scenercd;
    scenercd.shaderfilepath = "shaders/presentation.spv";
    scenercd.colorformat = m_device->defaultformat();
    scenercd.framesinflight = config.framesinflight;
    scenercd.layout = manager.getLayoutByName("presentation");
    m_scenerenderer = m_factory->createSceneRenderer(*m_device, scenercd);
}

void RenderSystem::update(Ecs& ecs, Lumos::ResourceManager& manager, float dt)
{
    auto frameinfo = m_device->startFrame();
    for (auto ent : ecs.view<Lumos::Camera>()) {
        auto& c = ecs.getComponent<Lumos::Camera>(ent);
        std::memcpy(manager.getBufferByNameAndIndex("global ubos", frameinfo.frameindex)->mappeddata(), &c, sizeof(c));
    }

    auto globalset = manager.getSetByNameAndIndex("global", frameinfo.frameindex);
    m_cuberenderer->begin(frameinfo, globalset);
    Lumos::Transform t;
    for (auto ent : ecs.view<Lumos::Size>()) {
        auto& s = ecs.getComponent<Lumos::Size>(ent);
        m_cuberenderer->submit(t, s.size, frameinfo);
    }
    m_cuberenderer->end(frameinfo, manager);

    // m_spriterenderer->begin(frameinfo, globalset);
    // for (auto ent : ecs.view<Lumos::Tile>()) {
    //     auto& t = ecs.getComponent<Lumos::Transform>(ent);
    //     auto& tile = ecs.getComponent<Lumos::Tile>(ent);
    //     m_spriterenderer->submit(t, tile, frameinfo);
    // }
    // m_spriterenderer->map(*manager.getBufferByNameAndIndex("sprite", frameinfo.frameindex));
    // frameinfo.set = manager.getSetByNameAndIndex("sprite", frameinfo.frameindex);
    // m_spriterenderer->end(frameinfo);

    m_scenerenderer->begin(frameinfo, globalset);
    frameinfo.set = manager.getSetByNameAndIndex("presentation", frameinfo.frameindex);
    m_scenerenderer->end(frameinfo, manager);
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
