#include "movementsystem.h"
#include <GLFW/glfw3.h>
#include "ecs/ecs.h"
#include "windowing/events.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

MovementSystem::MovementSystem(Lumos::EventBus& eventbus)
{
    eventbus.subscribe<Lumos::KeyPressEvent>([&](const Lumos::KeyPressEvent& e) {
        if (e.action == GLFW_PRESS) {
            m_heldkeys.insert(e.key);
        }
        if (e.action == GLFW_RELEASE) {
            m_heldkeys.erase(e.key);
        }
    });
    eventbus.subscribe<Lumos::CursorMoveEvent>([&](const Lumos::CursorMoveEvent& e) {
        double x = e.x;
        double u = e.y;
        std::println("cursor is moving: {}---{}", x, u);
    });

}

MovementSystem::~MovementSystem()
{

}

void MovementSystem::update(Ecs& ecs, float dt)
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
        auto& cuo = ecs.getComponent<Lumos::CameraUBO>(ent);
        auto& cam = ecs.getComponent<Lumos::Camera>(ent);

        float yaw = -90.0f;
        float pitch = 0.0f;
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        // cam.forward = glm::normalize(direction);
        t.position += cam.forward * dy * dt;
        t.position += glm::normalize(glm::cross(cam.forward, cam.up)) * dx * dt;
        std::println("forward: {}x{}x{}",cam.forward.x,cam.forward.y,cam.forward.z);
        auto view = lookAt(t.position, t.position + cam.forward, cam.up);
        auto proj = glm::perspective(glm::radians(45.0f),
            16/(float)9,
            cam.near, cam.far);
        proj[1][1] *= -1.0f;
        cuo.viewproj = proj * view;
    }
}

