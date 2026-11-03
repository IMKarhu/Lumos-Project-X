#pragma once
#include "textureatlas.h"
#include <filesystem>

class AtlasLoader
{
public:
    static TextureAtlas load(std::filesystem::path filepath);
private:
};
