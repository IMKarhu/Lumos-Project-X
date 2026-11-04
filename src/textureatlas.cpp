#include "textureatlas.h"
#include <stdexcept>

TextureAtlas::TextureAtlas(glm::ivec2 atlassize)
    : m_atlassize(atlassize)
{

}

void TextureAtlas::addRegion(const std::string& name, glm::ivec2 position, glm::ivec2 size)
{
    if (position.x < 0 || size.x <= 0 || position.y < 0 || size.y < 0 ||
        position.x + size.x > m_atlassize.x || position.y + size.y > m_atlassize.y) {
        throw std::invalid_argument("Atlas region is outside of atlas bounds");
    }
    m_atlasregions.insert_or_assign(name, AtlasRegion{.position = position, .size = size});
}

glm::vec4 TextureAtlas::uvRect(const std::string& name) const
{
    const auto& region = m_atlasregions.at(name);
    glm::vec2 uvmin = glm::vec2(region.position) / glm::vec2(m_atlassize);
    glm::vec2 uvmax = glm::vec2(region.position + region.size) / glm::vec2(m_atlassize);
    return {uvmin, uvmax};
}

const std::filesystem::path& TextureAtlas::texturePath() const
{
    return m_texturepath;
}

void TextureAtlas::setTexturePath(std::filesystem::path texturepath)
{
    m_texturepath = std::move(texturepath);
}
