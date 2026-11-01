#include "sampler.h"
#include "device.h"
#include "errorcheck.h"
#include <cassert>

namespace Lumos
{
    Sampler::Sampler(Device& device)
        : m_device(&device)
    {
        VkPhysicalDeviceProperties pdeviceproperties{};
        vkGetPhysicalDeviceProperties(m_device->pdevice(), &pdeviceproperties);

        VkSamplerCreateInfo samplerinfo {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = pdeviceproperties.limits.maxSamplerAnisotropy,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
        };
        VK_CHECK(vkCreateSampler(m_device->device(), &samplerinfo, nullptr, &m_sampler));
    }

    Sampler::~Sampler()
    {
        if (m_sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device->device(), m_sampler, nullptr);
        }
    }

    Sampler::Sampler(Sampler&& other) noexcept
        : m_device(other.m_device)
        , m_sampler(std::exchange(other.m_sampler, VK_NULL_HANDLE))
    {
    }

    Sampler& Sampler::operator=(Sampler&& other) noexcept
    {
        if (&other == this) {
            return *this;
        }
        if (m_sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device->device(), m_sampler, nullptr);
        }

        m_device = std::exchange(other.m_device, nullptr);
        m_sampler = std::exchange(other.m_sampler, VK_NULL_HANDLE);

        return *this;
    }

    VkSampler Sampler::sampler() const
    {
        return m_sampler;
    }
} //namespace Lumos
