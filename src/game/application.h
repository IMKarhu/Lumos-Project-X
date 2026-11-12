#pragma once
#include "engine/engine.h"
// #include "windowing/window.h"
// #include "windowing/events.h"
// #include "renderer/scene.h"
// #include "renderer/resourcemanager.h"
// #include "rendersystem.h"
// #include "movementsystem.h"
// #include "ecs/entity.h"
// #include "tilemap.h"
// #include <memory>

class Application
{
public:
    Application();
    void initEngine();
    void run();
    void initializeResources();
    void generatenoise();
private:
    Lumos::LumosEngine m_engine;
    // std::unique_ptr<Lumos::Window> m_window;
    // std::unique_ptr<RenderSystem> m_rendersystem;
    // std::unique_ptr<MovementSystem> m_movementsystem;
    // std::unique_ptr<Lumos::ResourceManager> m_resourcemanager;
    // Lumos::EventBus m_eventbus;
    // Lumos::EventQueue m_eventqueue;
    // Lumos::Scene m_scene;
    // std::string m_titleAndfps;
    // bool m_initialized = false;
    // TileMap m_tilemap{3, 3};
    // Lumos::VulkanConfig m_vkconfig{
    //     .framesinflight = 2,
    //     .validationlayers = true
    // };
    // std::vector<Lumos::Entity*> m_entities;
};
