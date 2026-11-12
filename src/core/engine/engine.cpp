#include "engine.h"

#include <print>
#include <stdexcept>
#include "resources/texture.h"

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
            auto tex = m_resourcemanager->load<Lumos::Texture>("witch_player_first");
            auto res = m_resourcemanager->getResource<Lumos::Texture>("witch_player_first");
            m_renderer->setTexture(res->image(), res->sampler());

            m_initialized = true;
        }
    }

    void LumosEngine::run()
    {
        if (!m_initialized) {
            throw std::runtime_error("Engine not initialized");
        }

        m_running = true;

        uint64_t framecount = 0;
        uint64_t lastfpsframe = 0;
        float timeaccumulator = 0.0f;
        float curfps = 0.0f;

        while(m_running)
        {
            m_window->pollEvents();
            if (m_window->shouldClose()) {
                m_running = false;
                break;
            }

            m_deltatime = deltatime();
            framecount++;
            timeaccumulator += m_deltatime.count() * 0.001f;

            if (timeaccumulator >= 1.0f) {
                uint64_t framessincelastupdate = framecount - lastfpsframe;
                double avgms = 0.0;
                if (framessincelastupdate > 0 && timeaccumulator > 0.0f) {
                    curfps = static_cast<float>(static_cast<double>(framessincelastupdate) / static_cast<double>(timeaccumulator));
                    avgms = (timeaccumulator / static_cast<double>(framessincelastupdate)) * 1000.0;

                } else {

                    curfps = std::max(curfps, 1.0f);
                    avgms = static_cast<double>(m_deltatime.count());
                }
                // std::string m_titleAndfps = std::format("Lumos engine test. Fps: {:.1f}", curfps);
                std::string m_titleAndfps = std::format("Lumos engine test. Fps: {} | avg Ms: {}",
                        static_cast<int>(curfps), static_cast<int>(avgms));
                m_window->setWindowTitle(m_titleAndfps);
            }

            update(m_deltatime);

            render();
        }
        m_renderer->waitIdle();
    }

    void LumosEngine::update(std::chrono::milliseconds dt)
    {
    }

    void LumosEngine::render()
    {
        if (!m_renderer) {
            return;
        }
        RenderData renderdata{};
        renderdata.cameradata.position = {0.0f, 0.0f};
        renderdata.cameradata.size = {640, 320};
        RenderObject obj;
        obj.position = {0.0f, 0.0f};
        obj.size = {100.0f, 100.0f};
        obj.rotation = 0.0f;
        obj.color = {1.0f, 0.0f, 0.0f, 1.0f};
        obj.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
        renderdata.renderobjects.push_back(obj);
        RenderObject obj2;
        obj2.position = {200.0f, 0.0f};
        obj2.size = {100.0f, 100.0f};
        obj2.rotation = 45.0f;
        obj2.color = {1.0f, 1.0f, 0.0f, 1.0f};
        obj2.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
        renderdata.renderobjects.push_back(obj2);
        RenderObject obj3;
        obj3.position = {0.0f, 100.0f};
        obj3.size = {100.0f, 100.0f};
        obj3.rotation = 90.0f;
        obj3.color = {0.5f, 1.0f, 1.0f, 1.0f};
        obj3.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
        renderdata.renderobjects.push_back(obj3);
        RenderObject obj4;
        obj4.position = {-200.0f, 0.0f};
        obj4.size = {100.0f, 100.0f};
        obj4.rotation = 90.0f;
        obj4.color = {0.0f, 0.0f, 1.0f, 1.0f};
        obj4.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
        renderdata.renderobjects.push_back(obj4);
        RenderObject obj5;
        obj5.position = {0.0f, -100.0f};
        obj5.size = {100.0f, 100.0f};
        obj5.rotation = 0.0f;
        obj5.color = {0.0f, 1.0f, 0.0f, 1.0f};
        obj5.uvrect = {0.0f, 0.0f, 1.0f, 1.0f};
        renderdata.renderobjects.push_back(obj5);
        m_renderer->render(renderdata);
    }

    const ResourceManager* LumosEngine::resourceManager() const
    {
        return m_resourcemanager.get();
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
