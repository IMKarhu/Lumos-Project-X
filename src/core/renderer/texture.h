#pragma once
#include "image.h"
#include "sampler.h"

namespace Lumos
{
    class Texture
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
