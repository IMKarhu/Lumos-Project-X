#pragma once
#include <unordered_set>

class Ecs;
namespace Lumos
{
    class EventBus;
}

class MovementSystem
{
public:
    MovementSystem(Lumos::EventBus& eventbus, Ecs& ecs);
    ~MovementSystem();
    void update(Ecs& ecs, float dt);
private:
    std::unordered_set<int> m_heldkeys;
    bool rightclickpressed = false;
    float cursorposx;
    float cursorposy;
};
