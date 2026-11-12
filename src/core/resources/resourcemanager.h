#pragma once
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cassert>
#include <print>
#include "renderer/renderer.h"
#include "resourcehandle.h"
#include "resource.h"

namespace Lumos
{
    class ResourceManager
    {
    public:
        ResourceManager(Renderer& renderer) :m_renderer(renderer) {}
        template<typename T, typename... Args>
        ResourceHandle<T> load(const std::string& id, Args&&...args) {
            static_assert(std::is_base_of<Resource, T>::value, "T must be derived from resource");
            auto& cache = m_resources[std::type_index(typeid(T))];
            if (auto it = cache.find(id); it != cache.end()) {
                if (auto existing = it->second) {
                    return ResourceHandle<T>(std::static_pointer_cast<T>(existing));
                }
            }

            std::shared_ptr<T> resource(new T(id, std::forward<Args>(args)...), [](T *r){
                    r->unload();
                    delete r;
                    });
            if (!resource->load(m_renderer)) {
                throw std::runtime_error("failed to load resource: " + id);
            } else {
                std::println("successfully loaded resource");
            }

            cache[id] = resource;
            return ResourceHandle<T>(resource);
        }

        template<typename T>
        ResourceHandle<T> getResource(const std::string& id) {
            auto typeit = m_resources.find(std::type_index(typeid(T)));
            if (typeit == m_resources.end()) {
                return {};
            }

            auto it = typeit->second.find(id);
            if (it == typeit->second.end()) {
                return {};
            }

            auto resource = it->second;
            if (!resource) {
                return {};
            }

            return ResourceHandle<T>(static_pointer_cast<T>(resource));
        }

        template<typename T>
        bool hasResource(const std::string& id) {
            return static_cast<bool>(getResource<T>(id));
        }

        //TODO: implement this
        template<typename T>
        void release(const std::string& id) {
            assert(0 && "TODO: IMPLEMENT ASSET RELEASE");
            // auto& refit = m_refcounts[std::type_index(typeid(T))];
            // auto it = refit.find(id);
            // if (it != refit.end()) {
            //     it->second.m_refcount--;
            // }
        }
    private:
        Renderer& m_renderer;
        std::unordered_map<std::type_index,
            std::unordered_map<std::string, std::shared_ptr<Resource>>> m_resources;
    };
} // namespace Lumos
