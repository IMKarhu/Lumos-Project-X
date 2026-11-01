#pragma once
#include "image.h"
#include "sampler.h"

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
    class CORE_API Texture
    {
    public:
        Texture(Image image, Sampler sampler);

        [[nodiscard]] const Image& image() const;
        [[nodiscard]] const Sampler& sampler() const;
    private:
        Image m_image;
        Sampler m_sampler;
    };
} //namespace Lumos
