#include "engine.h"

#include <print>
#include <stdexcept>
#include <algorithm>
#include "resources/textureatlas.h"

namespace Lumos
{
    LumosEngine::LumosEngine() {}

    void LumosEngine::initEngine(int width, int height, const std::string& name)
    {
        if (!m_initialized) {
            m_window = std::make_unique<Window>();
            // very much redundant check since if anything crucial fails in init. we throw runtime error
            // and otherwise we always return true
            if (!m_window->initWindow(width, height, name)) {
                m_initialized = false;
            } else {
                m_window->setWindowTitle(name);
                m_window->setResizeCallback([this](int width, int height) {
                        resize(width, height);
                });
                m_window->setScrollCallback([this](double x, double y) {
                        ScrollEvent event;
                        event.xoffset = x;
                        event.yoffset = y;
                        m_eventbus.publish(event);
                });
                m_initialized = true;
            }

            RendererConfig config {
                .framesinflight = 2,
                .validationlayers = true
            };
            m_renderer = std::make_unique<Renderer>(*m_window, config);

            m_resourcemanager = std::make_unique<ResourceManager>(*m_renderer);
            Lumos::AtlasDescription desc;
            desc.regions["test"] = {{0, 0}, {64, 32}};
            desc.regions["test2"] = {{64, 0}, {64, 32}};
            auto tex = m_resourcemanager->load<Lumos::TextureAtlas>("testsprite", "../testjsons/test.json");
            m_renderer->setTexture(tex->texture().image(), tex->texture().sampler());

            m_initialized = true;
        }
    }

    void LumosEngine::run()
    {
        if (!m_initialized) {
            throw std::runtime_error("Engine not initialized");
        }

        m_running = true;
    }

    void LumosEngine::update(double dt)
    {
    }

    void LumosEngine::render(Ecs& ecs)
    {
        if (!m_renderer) {
            return;
        }
        RenderData renderdata{};
        for (auto ent : ecs.view<Camera>()) {
            auto& cam = ecs.getComponent<Camera>(ent);
            auto& transform = ecs.getComponent<Transform>(ent);
            renderdata.cameradata.position = transform.position;
            renderdata.cameradata.size = transform.size;
            renderdata.cameradata.cameratype = cam.cameratype;
            renderdata.cameradata.rotation = cam.rotation;
            renderdata.cameradata.zoom = cam.zoom;
        }
        for (auto ent : ecs.view<Sprite>()) {
            auto& sprite = ecs.getComponent<Sprite>(ent);
            auto& transform = ecs.getComponent<Transform>(ent);
            RenderObject obj {
                .position = transform.position,
                .size = transform.size,
                .rotation = transform.rotation,
                .color = sprite.color,
                .uvrect = sprite.uvrect
            };
            renderdata.renderobjects.push_back(obj);
        }
        m_renderer->render(renderdata);
    }

    ResourceManager* LumosEngine::resourceManager() const
    {
        return m_resourcemanager.get();
    }

    Window* LumosEngine::window() const
    {
        return m_window.get();
    }

    bool LumosEngine::running() const
    {
        return m_running;
    }

    void LumosEngine::processEvents()
    {
        m_window->pollEvents();
        if (m_window->shouldClose()) {
            m_running = false;
        }
    }

    void LumosEngine::setTitle(const std::string& title)
    {
        m_window->setWindowTitle(title);
    }

    void LumosEngine::waitIdle()
    {
        m_renderer->waitIdle();
    }

    /********PRIVATE********/
    double LumosEngine::deltatime()
    {
        const auto cur = std::chrono::steady_clock::now();
        if (m_firstframe) {
            m_lastframe = cur;
            m_firstframe = false;
            return 0.0;
        }
        const std::chrono::duration<double> delta  = cur - m_lastframe;
        m_lastframe = cur;
        constexpr double maxdelta = 0.1;
        return std::min(delta.count(), maxdelta);
    }

    void LumosEngine::resize(int width, int height)
    {
        if (width <= 0 || height <= 0) {
            if (!m_window->shouldClose()) {
                return;
            }
            m_window->waitEvents();
        }
    }
} //namespace Lumos
