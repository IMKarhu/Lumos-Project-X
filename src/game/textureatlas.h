#pragma once
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <filesystem>

/*
 * Basically position is position in texture ie. first entry would be {0,0} second entry {64,0}
 * that is if textures are {64x32}
 * and size is the size of the sprite {64x32} should probably always be 2:1 ratio
 */
struct AtlasRegion
{
    glm::ivec2 position;
    glm::ivec2 size;
};

class TextureAtlas
{
public:
    /*
     * takes in the size of texture
     */
    TextureAtlas(glm::ivec2 atlassize);

    /*
     * inserts region into map or replaces a region if a region with that name already exists
     */
    void addRegion(const std::string& name, glm::ivec2 position, glm::ivec2 size);

    /*
     * calculates uvmin and uvmax and returns them
     */
    glm::vec4 uvRect(const std::string& name) const;

    /*
     * returns path to the texture atlas
     */
    const std::filesystem::path& texturePath() const;

    void setTexturePath(std::filesystem::path texturepath);
private:
    glm::ivec2 m_atlassize;
    std::unordered_map<std::string, AtlasRegion> m_atlasregions;
    std::filesystem::path m_texturepath;
};
