#include "rendersystem.h"
#include "windowing/window.h"
#include "renderer/device.h"
#include "resourcemanager.h"
#include "renderer/spriterenderer.h"
#include "ecs/ecs.h"
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

    m_spriteset = m_pool->allocateDescriptorSets(layouts, rcd.framesinflight);

    for (size_t i = 0; i < rcd.framesinflight; i++) {
        DescriptorWriter writer;
        writer.writeImage(0, m_testtexture->image(), m_testtexture->sampler());
        writer.writeBuffer(1, *m_spritebuffer, sprites.size() * sizeof(SpriteGPU), DescriptorWriter::STORAGE);
        writer.update(*m_device, m_spriteset[i]);
    }


    Lumos::SpriteRenderer::RendererConstructDescription rcd;
    rcd.shaderfilepath = "shaders/slang.spv";
    rcd.colorformat = m_device->defaultformat();
    rcd.framesinflight = config.framesinflight;
    rcd.globlayout = m_resourcemanager->getLayoutByName("global");
    m_spriterenderer = m_factory->createSpriteRenderer(*m_device, rcd);
}

RenderSystem::~RenderSystem() = default;

void RenderSystem::update(Ecs& ecs, float dt)
{
    auto frameinfo = m_device->startFrame();
    auto globalset = m_resourcemanager->getSetByNameAndIndex("global", frameinfo.frameindex);
    m_spriterenderer->begin(frameinfo, globalset);
    for (auto ent : ecs.view<Lumos::Transform>()) {
        // DO something
    }
    m_spriterenderer->end(frameinfo);
    m_device->endFrame(frameinfo);
}

void RenderSystem::waitForDestroy()
{
    m_device->waitForDestroy();
}
