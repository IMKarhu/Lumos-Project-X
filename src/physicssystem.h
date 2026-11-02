#pragma once
#include <unordered_set>

class Ecs;
namespace Lumos
{
    class EventBus;
}

class PhysicsSystem
{
public:
    PhysicsSystem(Lumos::EventBus& eventbus);
    ~PhysicsSystem();
    void update(Ecs& ecs, float dt);
private:
    std::unordered_set<int> m_heldkeys;
};
