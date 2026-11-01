#pragma once

class Ecs;
class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();
    void update(Ecs& ecs, float dt);
private:
};
