#pragma once
#include "resources/resourcemanager.h"
#include "windowing/window.h"
#include "windowing/events.h"
#include "renderer/renderer.h"
#include "logging.h"
#include "ecs/ecs.h"
#include <string>
#include <memory>
#include <chrono>

namespace Lumos
{
    class LumosEngine
    {
    public:
        LumosEngine();
        void initEngine(int width, int height, const std::string& name);
        void run();
        void update(std::chrono::milliseconds dt);
        void render(Ecs& ecs);

        ResourceManager* resourceManager() const;
        Window* window() const;
        std::chrono::milliseconds deltatime();
        bool running() const;
        void processEvents();

        void setTitle(const std::string& title);
        void waitIdle();
    private:
        bool m_initialized = false;
        bool m_running = true;
        std::chrono::milliseconds m_deltatime{0};
        uint64_t m_lastframeInMs = 0;

        std::unique_ptr<Window> m_window;
        std::unique_ptr<Renderer> m_renderer;
        std::unique_ptr<ResourceManager> m_resourcemanager;
        EventBus m_eventbus;

        void resize(int width, int height);
    };
} //namespace Lumos
