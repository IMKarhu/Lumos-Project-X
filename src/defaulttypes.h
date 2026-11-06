#pragma once
#include "renderer/vertex.h"
#include <vector>

namespace DefaultTypes
{
    namespace DefaultCube
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
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };
    } //namespace DefaultCube

} //namespace DefaultTypes
