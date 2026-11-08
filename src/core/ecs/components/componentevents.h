#pragma once
#include <vector>

namespace Lumos
{
    class ComponentEvent
    {
        virtual ~ComponentEvent() = default;
    };

    class ComponentEventListener
    {
    public:
        virtual ~ComponentEventListener() = default;
        virtual void onEvent(const ComponentEvent& event) = 0;
    };

    class ComponentEventSystem
    {
    public:
        void addListener(ComponentEventListener* listener) {
            m_listeners.push_back(listener);
        }

        void removeListener(ComponentEventListener* listener) {
            auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
            if (it != m_listeners.end()) {
                m_listeners.erase(it);
            }
        }

        void dispatch(const ComponentEvent& event) {
            for (auto listener : m_listeners) {
                listener->onEvent(event);
            }
        }
    private:
        std::vector<ComponentEventListener*> m_listeners;
    };
} //namespace Lumos
