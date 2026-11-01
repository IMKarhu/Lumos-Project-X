#pragma once
#include "renderer/baserenderer.h"
#include "renderer/renderfactory.h"
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
    RenderSystem(Lumos::Window& window);
    ~RenderSystem();
    void update(Ecs& ecs, float dt);
    void waitForDestroy();
private:
    std::unique_ptr<Lumos::Device> m_device;
    std::unique_ptr<Lumos::ResourceManager> m_resourcemanager;
    std::unique_ptr<Lumos::RenderFactory> m_factory;
    std::unique_ptr<Lumos::RendererBase> m_spriterenderer;

    struct UniformBuffer
    {
        glm::mat4 projectionview = glm::mat4(1);
        bool changed = false;
    };
};
