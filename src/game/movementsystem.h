#pragma once
#include <unordered_set>

namespace Lumos
{
    class EventBus;
}

class MovementSystem
{
public:
    MovementSystem(Lumos::EventBus& eventbus);
    ~MovementSystem();
    void update(float dt);
private:
    std::unordered_set<int> m_heldkeys;
    bool rightclickpressed = false;
    float cursorposx;
    float cursorposy;
};
