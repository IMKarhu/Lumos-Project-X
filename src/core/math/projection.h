#pragma once
#include <glm/glm.hpp>
#include <glm/gtc//matrix_transform.hpp>
#include "renderer/renderdata.h"

namespace Lumos
{
    namespace Math
    {
        inline glm::mat4 view(float rotation, glm::vec2 position)
        {
            glm::mat4 view;
            view = glm::rotate(glm::mat4(1.0f), -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
            view = view * glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
            return view;
        }

        inline glm::mat4 orthoProjection(glm::vec2 size, float zoom)
        {
            const glm::vec2 halfsize = size / (2.0f * zoom);
            return glm::ortho(
                    -halfsize.x,
                    halfsize.x,
                    -halfsize.y,
                    halfsize.y,
                    -1.0f,
                     1.0f);
        }

        inline glm::mat4 orthoViewProj(const CameraData& camera)
        {
            return orthoProjection(camera.size, camera.zoom) * view(camera.rotation, camera.position);
        }

        inline glm::mat4 inverseViewProj()
        {
            return glm::mat4(1.0f);
            // return glm::inverse()
        }

    } //namespace Math
} //namespace Lumos
