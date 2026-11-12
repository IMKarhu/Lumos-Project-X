#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace Lumos
{
    struct CameraData
    {
        glm::vec2 position;
        glm::vec2 size;
    };

    struct RenderObject
    {
        glm::vec2 position;
        glm::vec2 size;
        float rotation = 0.0f;
        glm::vec4 color;
        glm::vec4 uvrect{0.0f, 0.0f, 1.0f, 1.0f};
    };

    struct GpuEntity
    {
        glm::vec4 positionAndsize{};
        glm::vec4 rotationdata{};
        glm::vec4 color{1.0f};
        glm::vec4 uvrect{0.0f, 0.0f, 1.0f, 1.0f};
    };

    struct RenderData
    {
        CameraData cameradata;
        std::vector<RenderObject> renderobjects;
    };

    struct ObjectPushConstant
    {
        glm::vec2 position{};
        glm::vec2 size{1.0f};
        float rotation = 0.0f;
    };


} //namespace Lumos
