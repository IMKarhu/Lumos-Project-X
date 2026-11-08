#include "tilemap.h"

TileMap::TileMap(uint32_t width, uint32_t height)
    : m_width(width)
    , m_height(height)
    , m_tiles(static_cast<size_t>(width * height))
{
    for (size_t y = 0; y < m_height; ++y) {
        for (size_t x = 0; x < m_width; ++x) {
            m_tiles[indexOf({x, y})].position = {x, y};
        }
    }
}

void TileMap::addTile(Tile tile)
{
    m_tiles.push_back(tile);
}

bool TileMap::setTileType(glm::ivec2 position, TileType type)
{
    Tile* tile = tileAt(position);
    if (!tile) {
        return false;
    }
    tile->type = type;
    return true;
}

const std::vector<Tile>& TileMap::tiles() const
{
    return m_tiles;
}

Tile* TileMap::tileAt(glm::ivec2 gridposition)
{
    if (!contains(gridposition)) {
        return nullptr;
    }
    return &m_tiles[indexOf(gridposition)];
}

const Tile* TileMap::tileAt(glm::ivec2 gridposition) const
{
    if (!contains(gridposition)) {
        return nullptr;
    }
    return &m_tiles[indexOf(gridposition)];
}

glm::vec2 TileMap::tileToWorld(int col, int row) const
{
    return {(col - row) * 1.0f, (col + row) * 0.5f };
}

/*********PRIVATE*******/

bool TileMap::contains(glm::ivec2 position) const
{
    return position.x >= 0 && position.x < m_width
        && position.y >= 0 && position.y < m_height;
}

size_t TileMap::indexOf(glm::ivec2 position) const
{
    return static_cast<size_t>(position.y * m_width + position.x);
}
