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

    // struct Sprite
    // {
    //     //only a id handle to a texture
    //     uint32_t id;
    // };

    struct Camera
    {
        glm::vec3 upvector = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);
        float fov = 45.0f;
        float nearclip = 0.1f;
        float farclip = 100.0f;
        float aspectratio = 1.777f;
        bool primary = false;
    };

    struct Id
    {
        std::string name;
        uint32_t id;
    };
}// namespace Lumos
