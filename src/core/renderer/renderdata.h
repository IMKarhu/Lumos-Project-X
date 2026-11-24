#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace Lumos
{
    enum class CameraType
    {
        Orthographic,
        Perspective
    };
    struct CameraData
    {
        CameraType cameratype = CameraType::Orthographic;
        glm::vec2 position;
        glm::vec2 size;
        float zoom = 1.0f;
        float rotation = 0.0f;
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

    struct GridPushConstant
    {
        glm::vec4 majorcolor = {0.50f, 0.50f, 0.50f, 0.30f};
        glm::vec4 minorcolor = {0.60f, 0.60f, 0.60f, 0.60f};
        glm::vec4 cell = {1.0f, 10.0f, 8.0f, 0.0f}; //x = size, y = major every, z = fade start, w = fade end
    };
} //namespace Lumos
