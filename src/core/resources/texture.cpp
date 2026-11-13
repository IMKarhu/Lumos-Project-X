#include "texture.h"
#include "renderer/image.h"
#include "renderer/sampler.h"
#include "renderer/buffer.h"
#include "resourcemanager.h"
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

    const uint32_t Texture::width() const
    {
        return m_width;
    }

    const uint32_t Texture::height() const
    {
        return m_height;
    }

    bool Texture::loadResource(ResourceManager& manager)
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
        m_width = width;
        m_height = height;
        ktx_size_t imagesize = ktxTexture_GetImageSize(texture, 0);
        ktx_uint8_t* texturedata = ktxTexture_GetData(texture);
        if (!texturedata) {
            std::println("texture data is null");
            return false;
        }
        auto stage = manager.renderer().createBuffer(imagesize, texturedata);

        m_image = manager.renderer().createImage(width, height);

        m_sampler = manager.renderer().createSampler();
        manager.renderer().transitionImageLayout(m_image->image(),
                                         VK_FORMAT_R8G8B8A8_SRGB,
                                         VK_IMAGE_LAYOUT_UNDEFINED,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        manager.renderer().copyBufferToImage(*stage, m_image->image(), width, height);
        manager.renderer().transitionImageLayout(m_image->image(),
                               VK_FORMAT_R8G8B8A8_SRGB,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        ktxTexture_Destroy(texture);
        return true;
    }

    bool Texture::unloadResource()
    {
        // Do we need to "unload" anything here?
        // images and samplers destructor should be called automatically
        // when textures destructor is called
        return true;
    }
} //namespace Lumos
