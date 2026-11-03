#include "application.h"
#include "textureatlas.h"
#include "atlasloader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <format>

Application::Application()
{
    m_window = std::make_unique<Lumos::Window>(1280, 720, "test");
    m_window->setUserPtr(&m_eventbus);
    m_rendersystem = std::make_unique<RenderSystem>(*m_window);
    m_physicssystem = std::make_unique<PhysicsSystem>(m_eventbus);

    m_eventbus.subscribe<Lumos::KeyPressEvent>([&](const Lumos::KeyPressEvent& e) {
            if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
                m_window->setWindowShouldClose(true);
            }
        });


    Ecs& ecs = m_scene.ecs();
    Entity player = ecs.createEntity("player");
    Lumos::Transform trans;
    trans.position = glm::vec2(0.0f);
    trans.size = glm::vec2({20.0f, 10.0f});
    trans.rotation = 0.0f;
    Lumos::Camera camera;
    auto proj = glm::ortho(trans.position.x - trans.size.x * 0.5f,
                  trans.position.x + trans.size.x * 0.5f,
                  trans.position.y + trans.size.y * 0.5f,
                  trans.position.y - trans.size.y * 0.5f,
                  0.0f,
                  1.0f);
    // proj[1][1] *= -1;
    camera.viewproj = proj * glm::translate(glm::mat4(1.0f), glm::vec3(-trans.position, 0.0f));

    Lumos::InputState inputstate;
    inputstate.moved = true;
    ecs.addComponent(player, trans);
    ecs.addComponent(player, camera);
    ecs.addComponent(player, inputstate);
    bool testbool = true;
    auto atlas = AtlasLoader::load("../testjsons/test.json");
    // TextureAtlas atlas({128, 32});
    atlas.addRegion("floor", {0,0}, {64, 32});
    atlas.addRegion("floor2", {64,0}, {64, 32});
    for (size_t i = 0; i < m_tilemap.tiles().size(); i++) {
        Entity ent = ecs.createEntity("tile");
        Lumos::Transform t;
        t.position = IsometricProjection::gridToWorld(m_tilemap.tiles()[i].position);
        t.size = glm::vec2(2, 1);
        t.rotation = 0.0f;
        ecs.addComponent(ent, t);
        Lumos::Tile tile;
        if(testbool) {
            auto uvrect = atlas.uvRect("floor");
            tile.uvrect = uvrect;
            testbool = false;
        }
        else
        {
            auto uvrect = atlas.uvRect("floor2");
            tile.uvrect = uvrect;
            testbool = true;
        }
        ecs.addComponent(ent, tile);
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
