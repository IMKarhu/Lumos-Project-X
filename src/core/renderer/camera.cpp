#include "camera.h"

namespace Lumos
{
    void Camera2D::setPosition(const glm::vec2& position)
    {
        m_position = position;
    }

    void Camera2D::setSize(const glm::vec2& size)
    {
        m_size = size;
    }
        
    glm::mat4 Camera2D::projection() const
    {
        return glm::ortho(m_position.x - m_size.x * 0.5f,
                          m_position.x + m_size.x * 0.5f,
                          m_position.y + m_size.y * 0.5f,
                          m_position.y - m_size.y * 0.5f,
                          0.0f,
                          1.0f);
    }

    glm::mat4 Camera2D::view() const
    {
        return glm::translate(glm::mat4(1.0f), glm::vec3(-m_position, 0.0f));
    }
} //namespace Lumos
