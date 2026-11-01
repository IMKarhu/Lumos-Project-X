#pragma once
#include "ecs/ecs.h"

#include <string>
#include <string_view>

#ifdef _WIN32
    #ifdef CORE_EXPORT
    #define CORE_API __declspec(dllexport)
    #else
    #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API
#endif

namespace Lumos
{
    enum class GameState
    {
        MainMenu,
        Paused,
        Playing
    };

    class CORE_API Scene
    {
    public:
        Scene(const std::string_view name = "scene");
        [[nodiscard]] Ecs& ecs();
    private:
        Ecs m_ecs;
        std::string m_name;
        GameState type;
    };
} //namespace Lumos
