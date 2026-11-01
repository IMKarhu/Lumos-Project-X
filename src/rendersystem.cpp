#include "rendersystem.h"
#include "windowing/window.h"
#include "renderer/device.h"
#include "resourcemanager.h"
#include "renderer/spriterenderer.h"
#include "renderer/texture.h"
#include "ecs/ecs.h"
#include <glm/gtc/matrix_transform.hpp>

#include <array>



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
                sizeof(UniformBuffer),
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
                sizeof(UniformBuffer),
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
    m_resourcemanager->createTexture("sprite", "../textures/ground_dungeon7.ktx2");
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
    rcd.shaderfilepath = "shaders/slang.spv";
    rcd.colorformat = m_device->defaultformat();
    rcd.framesinflight = config.framesinflight;
    rcd.globlayout = m_resourcemanager->getLayoutByName("global");
    rcd.layout = m_resourcemanager->getLayoutByName("sprite");
    m_spriterenderer = m_factory->createSpriteRenderer(*m_device, rcd);
}

RenderSystem::~RenderSystem() = default;

void RenderSystem::update(Ecs& ecs, float dt)
{
    UniformBuffer ubo{};
    auto proj = glm::ortho(m_position.x - m_size.x * 0.5f,
                      m_position.x + m_size.x * 0.5f,
                      m_position.y + m_size.y * 0.5f,
                      m_position.y - m_size.y * 0.5f,
                      0.0f,
                      1.0f);
    // proj[1][1] *= -1;
    ubo.projectionview = proj * glm::translate(glm::mat4(1.0f), glm::vec3(-m_position, 0.0f));


        // ubo.projectionview = proj * m_camera.view();
        // std::memcpy(m_mappedubos[currentimage], &ubo, sizeof(ubo));

    auto frameinfo = m_device->startFrame();
    auto globalset = m_resourcemanager->getSetByNameAndIndex("global", frameinfo.frameindex);
    m_spriterenderer->begin(frameinfo, globalset);
    for (auto ent : ecs.view<Lumos::Transform>()) {
        auto& t = ecs.getComponent<Lumos::Transform>(ent);
        m_spriterenderer->submit(t, frameinfo);
    }
    m_spriterenderer->map(*m_resourcemanager->getBufferByNameAndIndex("sprite", frameinfo.frameindex));
    frameinfo.set = m_resourcemanager->getSetByNameAndIndex("sprite", frameinfo.frameindex);
    std::memcpy(m_resourcemanager->getBufferByNameAndIndex("global ubos", frameinfo.frameindex)->map(), &ubo, sizeof(ubo));
    m_resourcemanager->getBufferByNameAndIndex("global ubos", frameinfo.frameindex)->unmap();
    m_spriterenderer->end(frameinfo);
    m_device->endFrame(frameinfo);
}

void RenderSystem::waitForDestroy()
{
    m_device->waitForDestroy();
}

