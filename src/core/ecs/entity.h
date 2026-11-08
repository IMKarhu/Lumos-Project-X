#pragma once
#include "components/component.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace Lumos
{
    class Entity
    {
    public:
        explicit Entity(const std::string& name) : m_name(name) {}

        const std::string& name() const { return m_name; }
        bool active() const { return m_active; }
        void setActive(bool active) { m_active = active; }

        void initialize() {
            for (auto& component : m_components) {
                component->initialize();
            }
        }

        void update(float dt) {
            if (!m_active) {
                return;
            }

            for (auto& component : m_components) {
                component->update(dt);
            }
        }

        void render() {
            if (!m_active) {
                return;
            }

            for (auto& component : m_components) {
                component->render();
            }
        }

        template<typename T, typename... Args>
        T* addComponent(Args&&... args) {
            static_assert(std::is_base_of<Component, T>::value, "T must be derived of base component");
            size_t id = Component::typeID<T>();
            auto it = m_componentmap.find(id);
            if (it != m_componentmap.end()) {
                return static_cast<T*>(it->second);
            }

            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T* componentptr = component.get();
            m_componentmap[id] = componentptr;
            m_components.push_back(std::move(component));
            return componentptr;
        }

        template<typename T>
        T* getComponent() {
            size_t id = Component::typeID<T>();
            auto it = m_componentmap.find(id);
            if (it != m_componentmap.end()) {
                return static_cast<T*>(it->second);
            }
            return nullptr;
        }

        template<typename T>
        bool removeComponent() {
            size_t id = Component::typeID<T>();
            auto it = m_componentmap.find(id);
            if (it != m_componentmap.end()) {
                Component* componentptr = it->second;
                m_componentmap.erase(it);
                for (auto compit = m_components.begin(); compit != m_components.end(); ++compit) {
                    if (compit->get() == componentptr) {
                        m_components.erase(compit);
                        return true;
                    }
                }
            }
            return false;
        }
    private:
        std::string m_name;
        bool m_active = false;
        std::vector<std::unique_ptr<Component>> m_components;
        std::unordered_map<size_t, Component*> m_componentmap;
    };
} // namespace Lumos
