#pragma once
#include <string>
#include <memory>
#include <cassert>

namespace Lumos
{
    template<typename T>
    class ResourceHandle
    {
    public:
        ResourceHandle() = default;


        T* get() const {
            return m_resource.get();
        }

        T* operator->() const {
            return m_resource.get();
        }

        T& operator*() const {
            return *m_resource;
        }

        explicit operator bool() const {
            return m_resource != nullptr;
        }

        const std::string& id() const {
            assert(m_resource);
            return m_resource->id();
        }
    private:
        explicit ResourceHandle(std::shared_ptr<T> resource)
            : m_resource(std::move(resource)) {}

        std::shared_ptr<T> m_resource;
        friend class ResourceManager;
    };

} //namespace Lumos
