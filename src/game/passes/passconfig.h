#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <span>

struct PassConfig
{
    std::string shaderfilepath;
    VkFormat colorformat;
    std::span<const VkDescriptorSetLayout> layouts;
};
