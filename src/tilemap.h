#pragma once
#include <glm/glm.hpp>

enum class TileType
{
    Grass,
    Dirt,
    Stone
};

struct Tile
{
    uint32_t id;
    glm::vec2 position;
    TileType type;
};


class TileMap
{
public:
    TileMap(uint32_t width, uint32_t height);
    void addTile(Tile tile);
    bool setTileType(glm::ivec2 position, TileType type);
    const std::vector<Tile>& tiles() const;
    Tile* tileAt(glm::ivec2 gridposition);
    const Tile* tileAt(glm::ivec2 gridposition) const;
    glm::vec2 tileToWorld(int col, int row) const;
private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<Tile> m_tiles;

    bool contains(glm::ivec2 position) const;
    size_t indexOf(glm::ivec2 position) const;
};
