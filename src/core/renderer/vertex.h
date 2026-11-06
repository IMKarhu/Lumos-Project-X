#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Lumos
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        // glm::vec2 texcoord;

        static VkVertexInputBindingDescription bindingDescription()
        {
            return {.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
        }

        static std::vector<VkVertexInputAttributeDescription> attributeDescription()
        {
            std::vector<VkVertexInputAttributeDescription> description(2);
            description[0].binding = 0;
            description[0].location = 0;
            description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            description[0].offset = offsetof(Vertex, position);

            description[1].binding = 0;
            description[1].location = 1;
            description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            description[1].offset = offsetof(Vertex, color);

            // description[2].binding = 0;
            // description[2].location = 2;
            // description[2].format = VK_FORMAT_R32G32_SFLOAT;
            // description[2].offset = offsetof(Vertex, texcoord);
            return description;
        }

    };
} //namespace Lumos
