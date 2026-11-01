#include "application.h"
#include <chrono>
#include <format>

Application::Application()
{
    m_window = std::make_unique<Lumos::Window>(1080, 720, "test");
    m_rendersystem = std::make_unique<RenderSystem>(*m_window);
    m_physicssystem = std::make_unique<PhysicsSystem>();

    m_tiles = {
        {.position = {0, 0}, .type = TileType::Grass},
        {.position = {1, 0}, .type = TileType::Grass},
        {.position = {0, 1}, .type = TileType::Stone},
        {.position = {1, 1}, .type = TileType::Dirt},
    };
    Ecs& ecs = m_scene.ecs();
    for (size_t i = 0; i < 4; i++) {
        Entity ent = ecs.createEntity("tile");
        Lumos::Transform t;
        t.position = m_tiles[i].position;
        ecs.addComponent(ent, t);
    }
    for(auto ent : ecs.view<Lumos::Transform>()) {
        auto& t = ecs.getComponent<Lumos::Transform>(ent);
        std::println("position of entities: {}, {}", t.position.x, t.position.y);
    }
}

void Application::run()
{
    auto start = std::chrono::high_resolution_clock::now();

    float framecount = 0.0f;
    float timeaccumulator = 0.0f;
    float curfps = 0.0f;
    while(!m_window->shouldClose()) {
        m_window->pollEvents();
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
        m_physicssystem->update(m_scene.ecs(), dt);
        m_rendersystem->update(m_scene.ecs(), dt);
    }
    m_rendersystem->waitForDestroy();
}

namespace IsometricProjection
{
    glm::vec2 gridToWorld(glm::ivec2 gridposition)
    {
        return { gridposition.x - gridposition.y, (gridposition.x + gridposition.y) * 0.5f };
    }

}
