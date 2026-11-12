#pragma once
#include <memory>
#include "resource.h"
#include "renderer/image.h"

namespace Lumos
{
    class Device;
    class Sampler;
    class Texture : public Resource
    {
    public:
        explicit Texture(const std::string& id);
        ~Texture() override;

        [[nodiscard]] const Image& image() const;
        [[nodiscard]] const Sampler& sampler() const;

        bool loadResource(Renderer& renderer) override;
        bool unloadResource() override;
    private:
        void createImage(void* data, int with, int height, int channels);
        std::unique_ptr<Image> m_image;
        std::unique_ptr<Sampler> m_sampler;
    };
} //namespace Lumos
