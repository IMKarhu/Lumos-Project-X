#include "engine.h"

#include <print>
#include <stdexcept>
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
                m_window->setResizeCallback([this](int width, int height){
                        resize(width, height);
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
            auto res = m_resourcemanager->getResource<Lumos::TextureAtlas>("testsprite");
            m_renderer->setTexture(res->texture().image(), res->texture().sampler());

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

    void LumosEngine::update(std::chrono::milliseconds dt)
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
        // auto atlas = m_resourcemanager->getResource<Lumos::TextureAtlas>("testsprite");
        //
        // RenderData renderdata{};
        // renderdata.cameradata.position = {0.0f, 0.0f};
        // renderdata.cameradata.size = {640, 320};
        // RenderObject obj;
        // obj.position = {0.0f, 0.0f};
        // obj.size = {10.0f, 10.0f};
        // obj.rotation = 0.0f;
        // obj.color = {1.0f, 0.0f, 0.0f, 1.0f};
        // obj.uvrect = atlas->uv("floor2");
        // renderdata.renderobjects.push_back(obj);
        // RenderObject obj2;
        // obj2.position = {200.0f, 0.0f};
        // obj2.size = {100.0f, 100.0f};
        // obj2.rotation = 45.0f;
        // obj2.color = {1.0f, 1.0f, 0.0f, 1.0f};
        // obj2.uvrect = atlas->uv("floor");
        // renderdata.renderobjects.push_back(obj2);
        // RenderObject obj3;
        // obj3.position = {0.0f, 100.0f};
        // obj3.size = {100.0f, 100.0f};
        // obj3.rotation = 90.0f;
        // obj3.color = {0.5f, 1.0f, 1.0f, 1.0f};
        // obj3.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
        // renderdata.renderobjects.push_back(obj3);
        // RenderObject obj4;
        // obj4.position = {-200.0f, 0.0f};
        // obj4.size = {100.0f, 100.0f};
        // obj4.rotation = 90.0f;
        // obj4.color = {0.0f, 0.0f, 1.0f, 1.0f};
        // obj4.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
        // renderdata.renderobjects.push_back(obj4);
        // RenderObject obj5;
        // obj5.position = {0.0f, -100.0f};
        // obj5.size = {100.0f, 100.0f};
        // obj5.rotation = 0.0f;
        // obj5.color = {0.0f, 1.0f, 0.0f, 1.0f};
        // obj5.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
        // renderdata.renderobjects.push_back(obj5);
    }

    ResourceManager* LumosEngine::resourceManager() const
    {
        return m_resourcemanager.get();
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
    std::chrono::milliseconds LumosEngine::deltatime()
    {
        uint64_t curtime = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count());

        if (m_lastframeInMs == 0) {
            m_lastframeInMs = curtime;
            return std::chrono::milliseconds(16);
        }
        uint64_t delta  = curtime - m_lastframeInMs;
        m_lastframeInMs = curtime;
        return std::chrono::milliseconds(static_cast<long long>(delta));
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
