#pragma once
#include <vulkan/vulkan.h>

namespace Lumos
{
    enum class FilterMode
    {
        Linear,
        Nearest
    };

    enum class AddressMode
    {
        Repeat,
        MirrorRepeat,
        ClampToEdge
    };
    struct SamplerConfig
    {
        FilterMode minfilter = FilterMode::Nearest;
        FilterMode magfilter = FilterMode::Nearest;
        AddressMode addressmodeu = AddressMode::ClampToEdge;
        AddressMode addressmodev = AddressMode::ClampToEdge;
        AddressMode addressmodew = AddressMode::ClampToEdge;

        bool enableanisotropy = false;
    };
    class Device;
    class Sampler
    {
    public:
        Sampler(Device& device, SamplerConfig config);
        ~Sampler();

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;

        [[nodiscard]] VkSampler sampler() const;

    private:
        Device& m_device;
        VkSampler m_sampler = VK_NULL_HANDLE;
    };
} //namespace Lumos
