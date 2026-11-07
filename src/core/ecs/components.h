#pragma once
#include <glm/glm.hpp>
#include <string>

namespace Lumos
{
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 size;
        float rotation;
    };

    struct Camera
    {
        glm::vec3 wup; // global up vector ref
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); //local up direction
        glm::vec3 right; //local right direction
        glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
        float fov;
        float near;
        float far;
        float yaw = -90.0f;
        float pitch = 0.0f;
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
