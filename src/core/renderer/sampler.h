#pragma once
#include <vulkan/vulkan.h>

namespace Lumos
{
    class Device;
    class Sampler
    {
        public:
        Sampler(Device& device);

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
