#pragma once
#include <string>

namespace Lumos
{
    class Renderer;
    class Resource
    {
    public:
        explicit Resource(const std::string& id);
        virtual ~Resource() = default;

        const std::string& id() const;
        bool loaded() const;

        bool load(Renderer& renderer);
        void unload();
    protected:
        virtual bool loadResource(Renderer& renderer) = 0;
        virtual bool unloadResource() = 0;
    private:
        std::string m_id;
        bool m_loaded = false;
    };
} // namespace Lumos
