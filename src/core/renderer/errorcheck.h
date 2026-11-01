#pragma once
#include <print>
#include "vulkan/vk_enum_string_helper.h"

namespace Lumos
{
#define VK_CHECK(x) errorStatus(x)
    inline static void errorStatus(VkResult result)
    {
        if (result != VK_SUCCESS)
        {
            std::println("Vulkan error: {}", string_VkResult(result));
            std::abort();
        }
    }
} //namespace Lumos
