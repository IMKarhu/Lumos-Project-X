#include "sampler.h"
#include "device.h"
#include "errorcheck.h"
#include <cassert>
#include <utility>

namespace Lumos
{
    static VkFilter filterHelper(FilterMode filtermode)
    {
        switch(filtermode)
        {
            case Lumos::FilterMode::Linear:
                return VK_FILTER_LINEAR;
            case Lumos::FilterMode::Nearest:
                return VK_FILTER_NEAREST;
            default:
                return VK_FILTER_LINEAR;
        }
    }

    static VkBool32 anisotropy(bool enable)
    {
        if (enable) {
            return VK_TRUE;
        }
        return VK_FALSE;
    }

    Sampler::Sampler(Device& device, SamplerConfig config)
        : m_device(device)
    {
        VkPhysicalDeviceProperties pdeviceproperties{};
        vkGetPhysicalDeviceProperties(m_device.pdevice(), &pdeviceproperties);

        VkSamplerCreateInfo samplerinfo {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = filterHelper(config.filtermode),
            .minFilter = filterHelper(config.filtermode),
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .anisotropyEnable = anisotropy(config.enableanisotropy),
            .maxAnisotropy = pdeviceproperties.limits.maxSamplerAnisotropy,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
        };
        VK_CHECK(vkCreateSampler(m_device.device(), &samplerinfo, nullptr, &m_sampler));
    }

    Sampler::~Sampler()
    {
        if (m_sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device.device(), m_sampler, nullptr);
        }
    }

    VkSampler Sampler::sampler() const
    {
        return m_sampler;
    }
} //namespace Lumos
