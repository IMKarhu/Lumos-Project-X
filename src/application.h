#pragma once
#include "windowing/window.h"
#include "windowing/events.h"
#include "scene.h"
#include "resourcemanager.h"
#include "rendersystem.h"
#include "physicssystem.h"
#include "tilemap.h"
#include <memory>

class Application
{
public:
    Application();
    void run();
    void initializeResources();
    void generatenoise();
private:
    std::unique_ptr<Lumos::Window> m_window;
    std::unique_ptr<RenderSystem> m_rendersystem;
    std::unique_ptr<PhysicsSystem> m_physicssystem;
    std::unique_ptr<Lumos::ResourceManager> m_resourcemanager;
    Lumos::EventBus m_eventbus;
    Lumos::EventQueue m_eventqueue;
    Lumos::Scene m_scene;
    std::string m_titleAndfps;
    bool m_initialized = false;
    TileMap m_tilemap{3, 3};
    Lumos::VulkanConfig m_vkconfig{
        .framesinflight = 2,
        .validationlayers = true
    };
};

namespace IsometricProjection
{
    glm::vec2 gridToWorld(glm::ivec2 gridposition);
}
