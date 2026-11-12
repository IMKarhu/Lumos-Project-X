#include "resource.h"
#include "renderer/renderer.h"

namespace Lumos
{
    Resource::Resource(const std::string& id)
        : m_id(id) {}

    const std::string& Resource::id() const
    {
        return m_id;
    }

    bool Resource::loaded() const
    {
        return m_loaded;
    }

    bool Resource::load(Renderer& renderer)
    {
        if (m_loaded) {
            return true;
        }

        m_loaded = loadResource(renderer);
        return m_loaded;
    }

    void Resource::unload()
    {
        if (!m_loaded) {
            return;
        }
        if (unloadResource()) {
            m_loaded = false;
        }
    }
} //namespace Lumos
