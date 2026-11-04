#include "scene.h"

namespace Lumos
{
    Scene::Scene(const std::string_view name)
        : m_name(name)
    {

    }

    Ecs& Scene::ecs()
    {
        return m_ecs;
    }

    GameState Scene::gamestate() const
    {
        return m_gamestate;
    }
} //namespace Lumos
