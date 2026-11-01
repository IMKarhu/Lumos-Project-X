#include "texture.h"
#include <utility>

namespace Lumos
{
    Texture::Texture(Image image, Sampler sampler)
        : m_image(std::move(image))
        , m_sampler(std::move(sampler))
    {

    }

    const Image& Texture::image() const
    {
        return m_image;
    }

    const Sampler& Texture::sampler() const
    {
        return m_sampler;
    }

} //namespace Lumos
