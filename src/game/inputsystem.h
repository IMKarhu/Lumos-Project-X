#pragma once

namespace Lumos
{
    class EventBus;
}
class Ecs;
class InputSystem
{
public:
    InputSystem(Ecs& ecs);

    void setEventSystem(Lumos::EventBus& eventbus);
    void updateScrolling(double x, double y);
    void updatekeycalls(int key, bool pressed);
private:
    Ecs& m_ecs;
    Lumos::EventBus* m_eventbus;
};
