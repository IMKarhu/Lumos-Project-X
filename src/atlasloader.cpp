#include "atlasloader.h"
#include "core/json.hpp"
#include <fstream>
#include <stdexcept>

TextureAtlas AtlasLoader::load(std::filesystem::path filepath)
{
    std::ifstream file(filepath);
    if (!file) {
        throw std::runtime_error("failed to open atlas file");
    }

    nlohmann::json json;
    file >> json;
    auto size = json.at("size");

    TextureAtlas atlas({ size.at(0).get<int>(), size.at(1).get<int>() });

    auto path = json.at("texture").get<std::string>();
    atlas.setTexturePath(filepath.parent_path() / path);

    for (const auto& [name, region] : json.at("regions").items()) {
        auto position = region.at("position");
        auto dimensions = region.at("size");

        atlas.addRegion(name,
                { position.at(0).get<int>(), position.at(1).get<int>() },
                { dimensions.at(0).get<int>(), dimensions.at(1).get<int>() });
    }
    return atlas;
}
