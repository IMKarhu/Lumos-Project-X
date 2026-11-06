#pragma once
#include "renderer/vertex.h"
#include <vector>

namespace DefaultTypes
{
    namespace Cube
    {
        const std::vector<Lumos::Vertex> vertices = {
            { {-0.5f, -0.5f,  0.0f}, {1.0f, 0.0f, 0.0f} },
            { { 0.5f, -0.5f,  0.0f}, {0.0f, 1.0f, 0.0f} },
            { { 0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f} },
            { {-0.5f,  0.5f,  0.0f}, {1.0f, 1.0f, 1.0f} },

            { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f} },
            { { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f} },
            { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f} },
            { {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f} }

        };

        const std::vector<uint32_t> indices = {
            0, 1, 2, 0, 2, 3, // front
            4, 5, 6, 4, 6, 7, // back
            1, 5, 6, 1, 6, 2, // right
            0, 3, 7, 0, 7, 4, // left
            3, 7, 6, 3, 6, 2, // top
            0, 4, 5, 0, 5, 1  // bottom
            // 0, 1, 2, 2, 3, 0,
            // 4, 5, 6, 6, 7, 4
        };
    } //namespace DefaultCube

} //namespace DefaultTypes
