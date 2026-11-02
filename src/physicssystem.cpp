#include "physicssystem.h"
#include "ecs/ecs.h"
#include <print>

PhysicsSystem::PhysicsSystem()
{

}

PhysicsSystem::~PhysicsSystem()
{

}

void PhysicsSystem::update(Ecs& ecs, GLFWwindow* window, float dt)
{
    for (auto ent : ecs.view<Lumos::Camera>()) {
        // std::println("w is pressed");
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        std::println("w is pressed");
    }
    // std::println("hello from physics system");
}

