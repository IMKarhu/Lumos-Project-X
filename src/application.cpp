#include "application.h"
#include "ecs/components.h"
#include "resourcemanager.h"
#include "renderer/buffer.h"
#include "defaulttypes.h"

#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <chrono>
#include <format>
#include <stdexcept>

Application::Application()
{
    m_window = std::make_unique<Lumos::Window>(1280, 720, "test");
    m_window->setUserPtr(&m_eventbus);
    m_rendersystem = std::make_unique<RenderSystem>(*m_window, m_vkconfig);
    m_movementsystem = std::make_unique<MovementSystem>(m_eventbus);
    m_resourcemanager = std::make_unique<Lumos::ResourceManager>(*m_rendersystem->device());

    m_eventbus.subscribe<Lumos::KeyPressEvent>([&](const Lumos::KeyPressEvent& e) {
            if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
                m_window->setWindowShouldClose(true);
            }
        });


    Ecs& ecs = m_scene.ecs();
    Entity player = ecs.createEntity("player");
    Lumos::Transform trans;
    trans.position = glm::vec3(2.0f);
    trans.size = glm::vec2({10.0f, 10.0f});
    trans.rotation = 0.0f;
    Lumos::Camera camera;
    camera.forward = glm::vec3(-2.0f);
    camera.up = glm::vec3(0.0f, 0.0f, 1.0f);
    camera.far = 10.0f;
    camera.near = 0.1f;
    Lumos::CameraUBO camubo;
// static_cast<float>(m_window->width()) / static_cast<float>(m_window->height())
    auto view = lookAt(trans.position, trans.position + camera.forward, camera.up);
    auto proj = glm::perspective(glm::radians(45.0f),
            16/(float)9,
            camera.near, camera.far);
    proj[1][1] *= -1;
    camubo.viewproj = proj * view;
    ecs.addComponent(player, trans);
    ecs.addComponent(player, camera);
    ecs.addComponent(player, camubo);
    for (size_t i = 0; i < 10; i++) {
        Entity ent = ecs.createEntity("cube");
        Lumos::Transform t;
        t.size = glm::vec2(10, 10);
        t.rotation = 0.0f;
        t.position = glm::vec3(0.0f+i, 0.0f, 0.0f);
        ecs.addComponent(ent, t);
        Lumos::Model m;
        m.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0f));
        ecs.addComponent(ent, m);
        Lumos::Size indexsize;
        indexsize.size = static_cast<uint32_t>(DefaultTypes::Cube::indices.size());
        ecs.addComponent(ent, indexsize);
        std::println("creating cube");
    }
    initializeResources();
    m_rendersystem->createRenderers(*m_resourcemanager, m_vkconfig);
}

void Application::run()
{
    auto start = std::chrono::high_resolution_clock::now();

    float framecount = 0.0f;
    float timeaccumulator = 0.0f;
    float curfps = 0.0f;
// #if 0
    while(!m_window->shouldClose()) {
        m_window->pollEvents();
        m_eventqueue.process(m_eventbus);
        auto current = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current - start).count();
        start = current;

        timeaccumulator += dt;
        framecount += 1.0f;

        if (timeaccumulator >= 1.0f) {
            curfps = framecount / timeaccumulator;
            m_titleAndfps = std::format("Lumos engine test. Fps: {:.1f}", curfps);
            m_window->setWindowTitle(m_titleAndfps);
            framecount = 0.0f;
            timeaccumulator = 0.0f;
        }
        m_movementsystem->update(m_scene.ecs(), dt);
        m_rendersystem->update(m_scene.ecs(), *m_resourcemanager, dt);
    }
    m_rendersystem->waitForDestroy();
// #endif
}

void Application::initializeResources()
{
    const std::array globpool {
        Lumos::PoolConfig {
            .count = m_vkconfig.framesinflight,
            .type = Lumos::DescriptorType::Uniform
        }
    };
    const std::array globbindings {
        Lumos::BindingConfig {
            .binding = 0,
            .type = Lumos::DescriptorType::Uniform,
            .count = 1,
            .flags = Lumos::StageFlags::Vertex
        }
    };
    m_resourcemanager->create("global", globpool, globbindings, m_vkconfig.framesinflight);
    std::vector<Lumos::Buffer*> ubos;
    for (size_t i = 0; i < m_vkconfig.framesinflight; i++) {
        auto buf = new Lumos::Buffer(*m_rendersystem->device(),
                sizeof(Lumos::CameraUBO),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        buf->map();
        ubos.push_back(std::move(buf));
    }
    m_resourcemanager->cacheBuffers("global ubos", ubos);
    for (size_t i = 0; i < m_vkconfig.framesinflight; i++) {
        Lumos::DescriptorWriter writer;
        writer.writeBuffer(0, *m_resourcemanager->getBufferByNameAndIndex("global ubos", i),
                sizeof(Lumos::CameraUBO),
                Lumos::DescriptorWriter::UNIFORM);
        writer.update(*m_rendersystem->device(), m_resourcemanager->getSetByNameAndIndex("global", i));
    }
    // bool ResourceManager::createBuffer(const std::string& name, void* size, void* data, VkBufferUsageFlags type)

    VkDeviceSize size = sizeof(DefaultTypes::Cube::vertices[0]) * DefaultTypes::Cube::vertices.size();
    auto buffers = m_resourcemanager->createBuffer("cubevertex",
                                                  (void*)size,
                                                  (void*)DefaultTypes::Cube::vertices.data(),
                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT); 
    if (buffers.empty()) {
        throw std::runtime_error("failed to create a vertex buffer and vector is empty");
    }
    m_resourcemanager->cacheBuffers("cubevertex", buffers);

    size = sizeof(DefaultTypes::Cube::indices[0]) * DefaultTypes::Cube::indices.size();
    buffers = m_resourcemanager->createBuffer("cubeindex",
                                                  (void*)size,
                                                  (void*)DefaultTypes::Cube::indices.data(),
                                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    if (buffers.empty()) {
        throw std::runtime_error("failed to create a index buffer and vector is empty");
    }
    m_resourcemanager->cacheBuffers("cubeindex", buffers);


    //presentation stuff
    const std::array presentationpool {
        Lumos::PoolConfig {
            .count = m_vkconfig.framesinflight,
            .type = Lumos::DescriptorType::CombinedImageSampler
        },
    };
    const std::array presentationbindings {
        Lumos::BindingConfig {
            .binding = 0,
            .type = Lumos::DescriptorType::CombinedImageSampler,
            .count = 1,
            .flags = Lumos::StageFlags::Fragment
        },
    };
    m_resourcemanager->create("presentation", presentationpool, presentationbindings, m_vkconfig.framesinflight);
}

namespace IsometricProjection
{
    glm::vec2 gridToWorld(glm::ivec2 gridposition)
    {
        return { gridposition.x - gridposition.y, (gridposition.x + gridposition.y) * 0.5f };
    }

}
