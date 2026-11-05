#pragma once
#include "device.h"
#include "../ecs//components.h"
#include <vulkan/vulkan.h>

#ifdef _WIN32
    #ifdef CORE_EXPORT
    #define CORE_API __declspec(dllexport)
    #else
    #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API
#endif


namespace Lumos
{
    class Buffer;
    class Image;
    class Sampler;
    class ResourceManager;
    class CORE_API RendererBase
    {
    public:
        virtual ~RendererBase() = default;
        virtual void begin(Device::FrameInfo frameinfo, VkDescriptorSet globalset) = 0;
        virtual void end(Device::FrameInfo frameinfo, const ResourceManager& manager) = 0;
        virtual void submit(const Transform& t, uint32_t size, Device::FrameInfo frameinfo) = 0;
        virtual void map(Buffer& buffer) = 0;
        virtual Image* offscreenImage() const = 0;
        virtual Sampler* offscreenSampler() const = 0;
    };

} // namespace Lumos
