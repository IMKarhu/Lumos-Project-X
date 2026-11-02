#pragma once
#include "windowing/window.h"
#include "renderer/renderer.h"
#include "scene.h"
#include "rendersystem.h"
#include "physicssystem.h"
#include "tilemap.h"
#include <memory>

class Application
{
public:
    Application();
    void run();
private:
    std::unique_ptr<Lumos::Window> m_window;
    std::unique_ptr<RenderSystem> m_rendersystem;
    std::unique_ptr<PhysicsSystem> m_physicssystem;
    Lumos::Scene m_scene;
    std::string m_titleAndfps;
    TileMap m_tilemap{3, 3};
};

namespace IsometricProjection
{
    glm::vec2 gridToWorld(glm::ivec2 gridposition);
}
