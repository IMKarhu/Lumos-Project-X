#include "physicssystem.h"
#include <GLFW/glfw3.h>
#include "ecs/ecs.h"
#include "windowing/events.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

PhysicsSystem::PhysicsSystem(Lumos::EventBus& eventbus)
{
    eventbus.subscribe<Lumos::KeyPressEvent>([&](const Lumos::KeyPressEvent& e) {
            if (e.action == GLFW_PRESS) {
                m_heldkeys.insert(e.key);
            }
            if (e.action == GLFW_RELEASE) {
                m_heldkeys.erase(e.key);
            }
        });
}

PhysicsSystem::~PhysicsSystem()
{

}

void PhysicsSystem::update(Ecs& ecs, float dt)
{
    float dx = 0.0f;
    float dy = 0.0f;
    if (m_heldkeys.count(GLFW_KEY_W)) dy += 1.0f;
    if (m_heldkeys.count(GLFW_KEY_S)) dy -= 1.0f;
    if (m_heldkeys.count(GLFW_KEY_A)) dx -= 1.0f;
    if (m_heldkeys.count(GLFW_KEY_D)) dx += 1.0f;

    if (dx == 0.0f && dy == 0.0f) {
        return;
    }
    for (auto ent : ecs.view<Lumos::Camera>()) {
        auto& t = ecs.getComponent<Lumos::Transform>(ent);
        auto& i = ecs.getComponent<Lumos::InputState>(ent);
        t.position.y += dy * dt;
        t.position.x += dx * dt;
        // float pixelpwu = 640.0f / t.size.x;
        // glm::vec2 renderpos = t.position;
        // renderpos.x = std::round(t.position.x * pixelpwu) / pixelpwu;
        // renderpos.y = std::round(t.position.y * pixelpwu) / pixelpwu;
        auto& c = ecs.getComponent<Lumos::Camera>(ent);
        auto proj = glm::ortho(t.position.x - t.size.x * 0.5f,
                      t.position.x + t.size.x * 0.5f,
                      t.position.y + t.size.y * 0.5f,
                      t.position.y - t.size.y * 0.5f,
                      0.0f,
                      1.0f);
        c.viewproj = proj * glm::translate(glm::mat4(1.0f), glm::vec3(-t.position, 0.0f));
        i.moved = true;
    }
}

