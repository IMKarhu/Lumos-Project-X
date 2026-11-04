#include "rendersystem.h"
#include "windowing/window.h"
#include "renderer/device.h"
#include "resourcemanager.h"
#include "renderer/spriterenderer.h"
#include "renderer/scenerenderer.h"
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "ecs/ecs.h"
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cstring>

RenderSystem::RenderSystem(Lumos::Window& window)
{
    Lumos::VulkanConfig config {
        .framesinflight = 2,
        .validationlayers = true
    };
    m_device = std::make_unique<Lumos::Device>(window, config);
    m_resourcemanager = std::make_unique<Lumos::ResourceManager>(*m_device);
    m_factory = std::make_unique<Lumos::RenderFactory>();

    
    const std::array globpools {
        Lumos::PoolConfig {
            .count = config.framesinflight,
            .type = Lumos::DescriptorType::Uniform
        }
    };
    m_resourcemanager->createDescriptorPool("global", globpools, config.framesinflight);

    const std::array globbindings {
        Lumos::BindingConfig {
            .binding = 0,
            .type = Lumos::DescriptorType::Uniform,
            .count = 1,
            .flags = Lumos::StageFlags::Vertex
        }
    };
    m_resourcemanager->createDescriptorLayout("global", globbindings);

    std::vector<Lumos::Buffer*> ubos;
    for (size_t i = 0; i < config.framesinflight; i++) {
        auto buf = new Lumos::Buffer(*m_device,
                sizeof(Lumos::Camera),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        buf->map();
        ubos.push_back(std::move(buf));
    }
    m_resourcemanager->cacheBuffers("global ubos", ubos);
    m_resourcemanager->allocDescriptorSets("global", config.framesinflight);
    for (size_t i = 0; i < config.framesinflight; i++) {
        Lumos::DescriptorWriter writer;
        writer.writeBuffer(0, *m_resourcemanager->getBufferByNameAndIndex("global ubos", i),
                sizeof(Lumos::Camera),
                Lumos::DescriptorWriter::UNIFORM);
        writer.update(*m_device, m_resourcemanager->getSetByNameAndIndex("global", i));
    }


    //sprite resources
    const std::array spritepool {
        Lumos::PoolConfig {
            .count = config.framesinflight,
            .type = Lumos::DescriptorType::CombinedImageSampler
        },
            Lumos::PoolConfig {
            .count = config.framesinflight,
            .type = Lumos::DescriptorType::Storage
        },
    };
    m_resourcemanager->createDescriptorPool("sprite", spritepool, config.framesinflight);

    const std::array spritebindings {
        Lumos::BindingConfig {
            .binding = 0,
            .type = Lumos::DescriptorType::CombinedImageSampler,
            .count = 1,
            .flags = Lumos::StageFlags::Fragment
        },
        Lumos::BindingConfig {
            .binding = 1,
            .type = Lumos::DescriptorType::Storage,
            .count = 1,
            .flags = Lumos::StageFlags::Vertex
        }
    };
    m_resourcemanager->createDescriptorLayout("sprite", spritebindings);

    std::vector<Lumos::Buffer*> spritestorage;
    for (size_t i = 0; i < config.framesinflight; i++) {
        auto buf = new Lumos::Buffer(*m_device,
                MAXSPRITES * sizeof(Lumos::SpriteRenderer::SpriteGPU),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        spritestorage.push_back(std::move(buf));
    }
    m_resourcemanager->cacheBuffers("sprite", spritestorage);
    m_resourcemanager->createTexture("sprite", "../testjsons/testsprite.ktx2");
    // m_resourcemanager->createTexture("sprite", "../textures/witch_player_first.ktx2");
    m_resourcemanager->allocDescriptorSets("sprite", config.framesinflight);
    for (size_t i = 0; i < config.framesinflight; i++) {
        Lumos::DescriptorWriter writer;
        Lumos::Texture& tex = *m_resourcemanager->getTextureByName("sprite");
        writer.writeImage(0, tex.image(), tex.sampler());
        writer.writeBuffer(1, *m_resourcemanager->getBufferByNameAndIndex("sprite", i),
                MAXSPRITES * sizeof(Lumos::SpriteRenderer::SpriteGPU),
                Lumos::DescriptorWriter::STORAGE);
        writer.update(*m_device, m_resourcemanager->getSetByNameAndIndex("sprite", i));
    }

    Lumos::SpriteRenderer::RendererConstructDescription rcd;
    rcd.shaderfilepath = "shaders/sprite.spv";
    rcd.colorformat = m_device->defaultformat();
    rcd.framesinflight = config.framesinflight;
    rcd.globlayout = m_resourcemanager->getLayoutByName("global");
    rcd.layout = m_resourcemanager->getLayoutByName("sprite");
    m_spriterenderer = m_factory->createSpriteRenderer(*m_device, rcd);

    const std::array presentationpool {
        Lumos::PoolConfig {
            .count = config.framesinflight,
            .type = Lumos::DescriptorType::CombinedImageSampler
        },
    };
    m_resourcemanager->createDescriptorPool("presentation", presentationpool, config.framesinflight);

    const std::array presentationbindings {
        Lumos::BindingConfig {
            .binding = 0,
            .type = Lumos::DescriptorType::CombinedImageSampler,
            .count = 1,
            .flags = Lumos::StageFlags::Fragment
        },
    };


    m_resourcemanager->createDescriptorLayout("presentation", spritebindings);
    m_resourcemanager->allocDescriptorSets("presentation", config.framesinflight);
    for (size_t i = 0; i < config.framesinflight; i++) {
        Lumos::DescriptorWriter writer;
        auto img = m_spriterenderer->offscreenImage();
        auto sampler = m_spriterenderer->offscreenSampler();
        writer.writeImage(0, *img, *sampler);
        writer.update(*m_device, m_resourcemanager->getSetByNameAndIndex("presentation", i));
    }

    Lumos::SceneRenderer::RendererConstructDescription scenercd;
    scenercd.shaderfilepath = "shaders/presentation.spv";
    scenercd.colorformat = m_device->defaultformat();
    scenercd.framesinflight = config.framesinflight;
    scenercd.layout = m_resourcemanager->getLayoutByName("presentation");
    m_scenerenderer = m_factory->createSceneRenderer(*m_device, scenercd);
}

RenderSystem::~RenderSystem() = default;

void RenderSystem::update(Ecs& ecs, float dt)
{
    auto frameinfo = m_device->startFrame();
    std::memcpy(m_resourcemanager->getBufferByNameAndIndex("global ubos", frameinfo.frameindex)->mappeddata(), &m_uniformbuffer, sizeof(m_uniformbuffer));
    for (auto ent : ecs.view<Lumos::Camera>()) {
        auto& c = ecs.getComponent<Lumos::Camera>(ent);
        std::memcpy(m_resourcemanager->getBufferByNameAndIndex("global ubos", frameinfo.frameindex)->mappeddata(), &c, sizeof(c));
    }

    auto globalset = m_resourcemanager->getSetByNameAndIndex("global", frameinfo.frameindex);
    m_spriterenderer->begin(frameinfo, globalset);
    for (auto ent : ecs.view<Lumos::Tile>()) {
        auto& t = ecs.getComponent<Lumos::Transform>(ent);
        auto& tile = ecs.getComponent<Lumos::Tile>(ent);
        m_spriterenderer->submit(t, tile, frameinfo);
    }
    m_spriterenderer->map(*m_resourcemanager->getBufferByNameAndIndex("sprite", frameinfo.frameindex));
    frameinfo.set = m_resourcemanager->getSetByNameAndIndex("sprite", frameinfo.frameindex);
    m_spriterenderer->end(frameinfo);

    m_scenerenderer->begin(frameinfo, globalset);
    frameinfo.set = m_resourcemanager->getSetByNameAndIndex("presentation", frameinfo.frameindex);
    m_scenerenderer->end(frameinfo);
    m_device->endFrame(frameinfo);
}

void RenderSystem::waitForDestroy()
{
    m_device->waitForDestroy();
}

glm::vec2 RenderSystem::snapToPixelGrid(glm::vec2 position)
{
    constexpr float pixelsPerWorldUnit = 32.0f;
    return glm::round(position * pixelsPerWorldUnit)
     / pixelsPerWorldUnit;
}
