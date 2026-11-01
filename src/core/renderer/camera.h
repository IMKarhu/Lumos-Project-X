#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lumos
{
    class Camera2D
    {
    public:
        void setPosition(const glm::vec2& position);
        void setSize(const glm::vec2& size);
        
        [[nodiscard]] glm::mat4 projection() const;
        [[nodiscard]] glm::mat4 view() const;
    private:
        glm::vec2 m_position{0.0f};
        glm::vec2 m_size{10.0f, 10.0f};
    };
} //namespace Lumos
