#pragma once
#include <glm/glm.hpp>
#include <string>

namespace Lumos
{
    struct Transform
    {
        glm::vec3 position;
        glm::vec2 size;
        float rotation;
    };

    struct Camera
    {
        glm::mat4 viewproj = glm::mat4(1.0f);
    };

    //temporary struct for helping
    struct Tile
    {
        glm::vec4 uvrect;
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

    struct InputState
    {
        bool moved = false;
    };
}// namespace Lumos
