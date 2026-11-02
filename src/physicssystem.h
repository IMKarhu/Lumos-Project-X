#pragma once
#include "GLFW/glfw3.h"

class Ecs;
class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();
    void update(Ecs& ecs, GLFWwindow* window, float dt);
private:
};
