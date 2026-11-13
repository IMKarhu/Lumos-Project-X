#include "textureatlas.h"
#include "resourcemanager.h"

namespace Lumos
{
        TextureAtlas::TextureAtlas(const std::string& id, const AtlasDescription& desc)
            : Resource(id)
            , m_desc(desc) {}

        TextureAtlas::~TextureAtlas()
        {
            unload();
        }

        const Texture& TextureAtlas::texture() const
        {
            return *m_texture;
        }

        glm::vec4 TextureAtlas::uv(const std::string& name) const
        {
            auto it = m_regions.find(name);
            return it->second;
        }

        bool TextureAtlas::loadResource(ResourceManager& manager)
        {
            m_texture = manager.load<Texture>(id());

            for (const auto& [name, region] : m_desc.regions) {
                const glm::vec4 uv = {
                    region.position.x / m_desc.texturesize.x,
                    region.position.y / m_desc.texturesize.y,
                    region.size.x / m_desc.texturesize.x,
                    region.size.y / m_desc.texturesize.y
                };
                m_regions.emplace(name, uv);
            }
            return true;
        }

        bool TextureAtlas::unloadResource()
        {
            m_regions.clear();
            m_texture = {};
            return true;
        }

} // namespace Lumos
