#include "texture.h"
#include "renderer/image.h"
#include "renderer/sampler.h"
#include "renderer/buffer.h"
#include "renderer/renderer.h"
#include "ktx.h"
#include <utility>
#include <print>

namespace Lumos
{
    Texture::Texture(const std::string& id)
        : Resource(id) {}


    Texture::~Texture()
    {
        unload();
    }

    const Image& Texture::image() const
    {
        return *m_image;
    }
    const Sampler& Texture::sampler() const
    {
        return *m_sampler;
    }

    bool Texture::loadResource(Renderer& renderer)
    {
        std::string filepath = "../textures/"+id()+".ktx2";
        ktxTexture* texture = nullptr;
        const KTX_error_code ktxresult = ktxTexture_CreateFromNamedFile(filepath.c_str(),
                KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                &texture);

        if (ktxresult != KTX_SUCCESS) {
            std::println("failed to load ktx texture: {}", ktxErrorString(ktxresult));
            return false;
        }
        std::println("texture size: {}x{}", texture->baseWidth, texture->baseHeight);
        uint32_t width = texture->baseWidth;
        uint32_t height = texture->baseHeight;
        ktx_size_t imagesize = ktxTexture_GetImageSize(texture, 0);
        ktx_uint8_t* texturedata = ktxTexture_GetData(texture);
        if (!texturedata) {
            std::println("texture data is null");
            return false;
        }
        auto stage = renderer.createBuffer(imagesize, texturedata);

        m_image = renderer.createImage(width, height);

        m_sampler = renderer.createSampler();
        renderer.transitionImageLayout(m_image->image(),
                                         VK_FORMAT_R8G8B8A8_SRGB,
                                         VK_IMAGE_LAYOUT_UNDEFINED,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        renderer.copyBufferToImage(*stage, m_image->image(), width, height);
        renderer.transitionImageLayout(m_image->image(),
                               VK_FORMAT_R8G8B8A8_SRGB,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        ktxTexture_Destroy(texture);
        return true;
    }

    bool Texture::unloadResource()
    {
        return false;
    }

    void Texture::createImage(void* data, int with, int height, int channels)
    {
    }
} //namespace Lumos
