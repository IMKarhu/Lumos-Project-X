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

    static VkSamplerAddressMode addressmodeHelper(AddressMode addressmode)
    {
        switch(addressmode)
        {
            case Lumos::AddressMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case Lumos::AddressMode::MirrorRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case Lumos::AddressMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            default:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    }

    Sampler::Sampler(Device& device, SamplerConfig config)
        : m_device(device)
    {
        VkPhysicalDeviceProperties pdeviceproperties{};
        vkGetPhysicalDeviceProperties(m_device.pdevice(), &pdeviceproperties);

        VkSamplerCreateInfo samplerinfo {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = filterHelper(config.magfilter),
            .minFilter = filterHelper(config.minfilter),
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = addressmodeHelper(config.addressmodeu),
            .addressModeV = addressmodeHelper(config.addressmodev),
            .addressModeW = addressmodeHelper(config.addressmodew),
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
