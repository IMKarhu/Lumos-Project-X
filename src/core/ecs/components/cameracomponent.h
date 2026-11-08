#pragma once
#include "component.h"
#include "componentevents.h"
#include "transformcomponent.h"
#include "../entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lumos
{
    class Camera : public Component, public ComponentEventListener
    {
    public:
        void onInitialize() override {
            eventSystem().addListener(this);
        }

        ~Camera() override {
            eventSystem().removeListener(this);
        }

        void onEvent(const ComponentEvent& event) override {
        }
        void setPerspective(float fov, float aspectratio, float near, float far) {
            m_fov = fov;
            m_aspectratio = aspectratio;
            m_near = near;
            m_far = far;
            m_dirty = true;
        }
        
        glm::mat4 viewMatrix() const {
            auto transform = getOwner()->getComponent<Transform>();
            if (transform) {
                glm::vec3 position = transform->position();
                glm::quat rotation = transform->rotation();
                glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
                glm::vec3 up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);
                return glm::lookAt(position, position + forward, up);
            }
            return glm::mat4(1.0f);
        }

        glm::mat4 projectionMatrix() const {
            if (m_dirty) {
                m_projectionmatrix = glm::perspective(glm::radians(m_fov),
                        m_aspectratio, m_near, m_far);
                m_dirty = false;
            }
            return m_projectionmatrix;
        }
    private:
        float m_fov = 45.0f;
        float m_near = 0.1f;
        float m_far = 100.0f;
        float m_aspectratio = 16.0f / 9.0f;
        glm::mat4 m_viewmatrix = glm::mat4(1.0f);
        mutable glm::mat4 m_projectionmatrix = glm::mat4(1.0f);
        mutable bool m_dirty = true;

        ComponentEventSystem& eventSystem() {
            static ComponentEventSystem eventsystem;
            return eventsystem;
        }
    };

} //namespace Lumos
