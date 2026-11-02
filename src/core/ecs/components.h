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
        glm::mat4 viewproj = glm::mat4(1.0f);
    };

    //temporary struct for helping
    struct Tile
    {
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
