#pragma once
#include "component.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lumos
{
    class Transform : public Component
    {
    public:
        const glm::vec3& position() const { return m_position; }
        void setPosition(const glm::vec3& pos) {
            m_position = pos;
            m_dirty = true;
        }

        const glm::quat& rotation() const { return m_rotation; }
        void setRotation(const glm::quat& rot) {
            m_rotation = rot;
            m_dirty = true;
        }

        const glm::vec3 scale() const { return m_scale; }
        void setScale(const glm::vec3& scale) {
            m_scale = scale;
            m_dirty = true;
        }

        glm::mat4 transformMatrix() const {
            if (m_dirty) {
                glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
                glm::mat4 rotation = glm::mat4_cast(m_rotation);
                glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);
                m_modelmatrix = translation * rotation * scale;
                m_dirty = false;
            }
            return m_modelmatrix;
        }
    private:
        glm::vec3 m_position = glm::vec3(0.0f);
        glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 m_scale = glm::vec3(1.0f);
        mutable glm::mat4 m_modelmatrix = glm::mat4(1.0f);
        mutable bool m_dirty = true;
    };
} //namespace Lumos
