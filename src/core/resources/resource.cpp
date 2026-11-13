#include "resource.h"
#include "resourcemanager.h"

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

    bool Resource::load(ResourceManager& manager)
    {
        if (m_loaded) {
            return true;
        }

        m_loaded = loadResource(manager);
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
