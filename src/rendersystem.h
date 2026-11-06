#pragma once
#include "renderer/baserenderer.h"
// #include "renderer/renderfactory.h"
#include "passes/scenepass.h"
#include "passes/terrainpass.h"
#include <memory>

namespace Lumos
{
    class Device;
    class ResourceManager;
    class Window;
}

class Ecs;
class RenderSystem
{
public:
    RenderSystem(Lumos::Window& window, Lumos::VulkanConfig config);
    ~RenderSystem();
    void createRenderers(Lumos::ResourceManager& manager, Lumos::VulkanConfig config);
    void update(Ecs& ecs, Lumos::ResourceManager& manager, float dt);
    void waitForDestroy();
    Lumos::Device* device()const;
    glm::vec2 snapToPixelGrid(glm::vec2 position);

private:
    std::unique_ptr<Lumos::Device> m_device;
    // std::unique_ptr<Lumos::RenderFactory> m_factory;
    // std::unique_ptr<Lumos::RendererBase> m_spriterenderer;
    // std::unique_ptr<Lumos::DRenderer> m_cuberenderer;
    // std::unique_ptr<Lumos::RendererBase> m_scenerenderer;
    TerrainPass m_terrainpass;
    ScenePass m_scenepass;
    // constexpr static std::size_t MAXSPRITES = 1000;
};
