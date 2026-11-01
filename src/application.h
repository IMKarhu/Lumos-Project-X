#pragma once
#include "windowing/window.h"
#include "renderer/renderer.h"
#include "scene.h"
#include "rendersystem.h"
#include "physicssystem.h"
#include <memory>

enum class TileType
{
    Grass,
    Dirt,
    Stone
};

struct Tile
{
    glm::vec2 position;
    TileType type;
};

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
    std::vector<Tile> m_tiles;
};

namespace IsometricProjection
{
    glm::vec2 gridToWorld(glm::ivec2 gridposition);
}
