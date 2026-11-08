#pragma once

#include <string>
#include <string_view>

namespace Lumos
{
    enum class GameState
    {
        MainMenu,
        Paused,
        Playing
    };

    class Scene
    {
    public:
        Scene(const std::string_view name = "scene");
        [[nodiscard]] GameState gamestate() const;
    private:
        std::string m_name;
        GameState m_gamestate;
    };
} //namespace Lumos
