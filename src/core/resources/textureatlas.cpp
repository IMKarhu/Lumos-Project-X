#include "textureatlas.h"
#include "resourcemanager.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <utility>

namespace Lumos
{
        TextureAtlas::TextureAtlas(const std::string& id, const std::filesystem::path& filepath)
            : Resource(id)
            , m_filepath(filepath) {}

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
            std::ifstream file(m_filepath);
            if (!file.is_open()) {
                return false;
            }
            nlohmann::json json;

            try
            {
                file >> json;
                const std::string texpath = json.at("texture").get<std::string>();
                m_texture = manager.load<Texture>(id());
                const float width = static_cast<float>(m_texture->width());
                const float height = static_cast<float>(m_texture->height());

                for (const auto& [name, region] : json.at("regions").items()) {
                    const auto position = region.at("position");
                    const auto size = region.at("size");
                    AtlasRegion atlasregion {
                        { position[0].get<float>(), position[1].get<float>()},
                        { size[0].get<float>(), size[1].get<float>()}
                    };
                    // if (region.size.x <= 0.0f || region.size.y <= 0.0f) {
                    //     throw std::runtime_error("Invalid atlas region size" + name);
                    // }
                    const glm::vec4 uv = {
                        atlasregion.position.x / width,
                        atlasregion.position.y / height,
                        atlasregion.size.x / width,
                        atlasregion.size.y / height
                    };
                    m_regions.emplace(name, uv);
                }
            }
            catch(const std::exception& e) {
                std::cerr << "Failed to load texture atlas: " << m_filepath << ":" << e.what() << "\n";
                return false;
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
