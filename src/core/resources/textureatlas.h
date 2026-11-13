#pragma once
#include <glm/glm.hpp>
#include "resource.h"
#include "resourcehandle.h"
#include "texture.h"

namespace Lumos
{
    struct AtlasRegion
    {
        glm::vec2 position{};
        glm::vec2 size{};
    };

    struct AtlasDescription
    {
        glm::uvec2 texturesize{};
        std::unordered_map<std::string, AtlasRegion> regions;
    };

    class TextureAtlas : public Resource
    {
    public:
        explicit TextureAtlas(const std::string& id, const AtlasDescription& desc);
        ~TextureAtlas() override;

        const Texture& texture() const;
        glm::vec4 uv(const std::string& name) const;

        bool loadResource(ResourceManager& manager) override;
        bool unloadResource() override;
    private:
        AtlasDescription m_desc;
        ResourceHandle<Texture> m_texture;
        std::unordered_map<std::string, glm::vec4> m_regions;
    };
} //namespace Lumos
