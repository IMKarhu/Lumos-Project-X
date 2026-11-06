#pragma once
#include <vulkan/vulkan.h>

namespace Lumos
{
    enum class FilterMode
    {
        Linear,
        Nearest
    };
    struct SamplerConfig
    {
        FilterMode filtermode = FilterMode::Nearest;
        bool enableanisotropy = false;
    };
    class Device;
    class Sampler
    {
        public:
        Sampler() = default;
        Sampler(Device& device, SamplerConfig config);

        ~Sampler();

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;

        Sampler(Sampler&& other) noexcept;
        Sampler& operator=(Sampler&& other) noexcept;

        [[nodiscard]] VkSampler sampler() const;

        private:
        Device* m_device = nullptr;
        VkSampler m_sampler = VK_NULL_HANDLE;
    };
} //namespace Lumos
