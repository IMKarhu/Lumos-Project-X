#pragma once
#include <glm/glm.hpp>
#include <string>

namespace Lumos
{
    struct Transform
    {
        glm::vec2 position;
        glm::vec2 size;
        float rotation;
    };

    struct Camera
    {
        float zoom = 1.0f;
        float rotation = 0.0f;
    };

    struct Sprite
    {
        glm::vec4 color;
        glm::vec4 uvrect;
    };

    struct CameraUBO
    {
        glm::mat4 viewproj = glm::mat4(1.0f);
    };

    struct Model
    {
        glm::mat4 model = glm::mat4(1.0f);
    };

    struct Size
    {
        uint32_t size;
    };

    struct Id
    {
        std::string name;
        uint32_t id;
    };
}// namespace Lumos
