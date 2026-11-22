#pragma once
#include <print>
#include "engine/logging.h"
#include "vulkan/vk_enum_string_helper.h"

namespace Lumos
{
#define VK_CHECK(x) errorStatus(x)
    inline static bool errorStatus(VkResult result)
    {
        if (result != VK_SUCCESS)
        {
            ConsoleLogger logger;
            std::string message = std::format("Vulkan error: {}",string_VkResult(result));
            LS_LOG_ERROR(logger, message);
            return false;
        }
        return true;
    }
} //namespace Lumos
