#include "inputsystem.h"
#include <algorithm>
#include <cmath>
#include "ecs/ecs.h"
#include "windowing/events.h"

InputSystem::InputSystem(Ecs& ecs)
    : m_ecs(ecs) {

    }

void InputSystem::setEventSystem(Lumos::EventBus& eventbus)
{
    m_eventbus = &eventbus;
    m_eventbus->subscribe<Lumos::ScrollEvent>([&](const Lumos::ScrollEvent& e) {
            updateScrolling(e.xoffset, e.yoffset);
    });
}

void InputSystem::updateScrolling(double x, double y)
{
    constexpr float step = 1.1f;
    constexpr float min = 0.1f;
    constexpr float max = 10.0f;
    for (auto ent : m_ecs.view<Lumos::Camera>()) {
        auto& cam = m_ecs.getComponent<Lumos::Camera>(ent);
        cam.zoom *= std::pow(step, static_cast<float>(y));
        cam.zoom = std::clamp(cam.zoom, min, max);
    }
}

void InputSystem::updatekeycalls(int key, bool pressed)
{
    for (auto ent : m_ecs.view<Lumos::Transform>()) {
        auto& id = m_ecs.getComponent<Lumos::Id>(ent);
        if (id.name == "player") {
            auto& transform = m_ecs.getComponent<Lumos::Transform>(ent);


        }
    }
}

