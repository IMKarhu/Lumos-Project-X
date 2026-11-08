#pragma once
#include "renderer/baserenderer.h"
#include "passes/scenepass.h"
#include "passes/terrainpass.h"
#include <memory>

namespace Lumos
{
    class Device;
    class ResourceManager;
    class Window;
}

class RenderSystem
{
public:
    RenderSystem(Lumos::Window& window, Lumos::VulkanConfig config);
    ~RenderSystem();
    void createRenderers(Lumos::ResourceManager& manager, Lumos::VulkanConfig config);
    void update(Lumos::ResourceManager& manager, float dt);
    void waitForDestroy();
    Lumos::Device* device()const;
private:
    std::unique_ptr<Lumos::Device> m_device;
    TerrainPass m_terrainpass;
    ScenePass m_scenepass;
};
