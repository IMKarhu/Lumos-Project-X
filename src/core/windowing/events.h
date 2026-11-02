#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <typeindex>
#include <queue>
#include <print>

namespace Lumos
{

    class EventBus
    {
    private:
        struct IHandlerList
        {
            virtual ~IHandlerList() = default;
        };
        template<typename T>
        struct HandlerList : public IHandlerList
        {
            std::vector<std::function<void(const T&)>> handlers;
        };
    public:
        template<typename EventType>
        void subscribe(std::function<void(const EventType&)> callback)
        {
            auto typeindex = std::type_index(typeid(EventType));
            if (m_subscribers.find(typeindex) == m_subscribers.end()) {
                m_subscribers[typeindex] = std::make_unique<HandlerList<EventType>>();
            }
            auto* list = static_cast<HandlerList<EventType>*>(m_subscribers[typeindex].get());
            list->handlers.push_back(callback);
        }

        template<typename EventType>
        void publish(const EventType& event) {
            // std::println("publish in eventbus");
            auto typeindex = std::type_index(typeid(EventType));
            auto it = m_subscribers.find(typeindex);
            if (it != m_subscribers.end()) {
                auto* list = static_cast<HandlerList<EventType>*>(it->second.get());
                for (const auto& handler : list->handlers) {
                    handler(event);
                }
            }
        }
    private:
        std::unordered_map<std::type_index, std::unique_ptr<IHandlerList>> m_subscribers;
    };

    struct Event
    {
        virtual ~Event() = default;
        virtual void dispatch(EventBus& bus) const = 0;
    };

    struct KeyPressEvent : public Event
    {
        int key;
        int scancode;
        int action;
        int mods;
        void dispatch(EventBus& bus) const override
        {
            bus.publish(*this);
        }
    };

    struct ResizeEvent : public Event
    {
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        bool m_framebufferresized = false;
    };

    class EventQueue
    {
    public:
        template<typename EventType, typename... Args>
        void emit(Args&&... args) {
            std::println("emitted");
            m_queue.push(std::make_unique<EventType>(std::forward<Args>(args)...));
        }

        void process(EventBus& bus) {
            while (!m_queue.empty()) {
                m_queue.front()->dispatch(bus);
                m_queue.pop();
            }
        }
    private:
        std::queue<std::unique_ptr<Event>> m_queue;
    };
} //namespace Lumos
