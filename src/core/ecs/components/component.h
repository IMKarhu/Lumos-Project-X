#pragma once
#include "componenttypeid.h"

namespace Lumos
{
    /*
     * Forward declarations
     */
    class Entity;
    size_t ComponentTypeID::m_nexttypeid = 0;

    class Component
    {
    public:
        enum class EntityState {
            Uninitialized,
            PendingForInitialisation,
            Initialized,
            Active,
            PendingForDestruction,
            Destroyed
        };
        virtual ~Component() {
            if (m_state != EntityState::Destroyed) {
                onDestroy();
                m_state = EntityState::Destroyed;
            }
        }

        template<typename T>
        static size_t typeID() {
            return ComponentTypeID::typeID<T>();
        }

        void initialize() {
            if (m_state == EntityState::Uninitialized) {
                m_state = EntityState::PendingForInitialisation;
                onInitialize();
                m_state = EntityState::Initialized;
            }
        }

        void destroy() {
            if (m_state == EntityState::Active) {
                m_state = EntityState::PendingForDestruction;
                onDestroy();
                m_state = EntityState::Destroyed;
            }

        }

        bool active() const { return m_state == EntityState::Active; }

        virtual void setOwner(Entity* entity) { m_owner = entity; }
        Entity* getOwner() const { return m_owner; }
    protected:
        virtual void onInitialize() {}
        virtual void onDestroy() {}
        virtual void update(float dt) {}
        virtual void render() {}

        friend class Entity;
    private:
        EntityState m_state = EntityState::Uninitialized;
        Entity* m_owner;
    };
} //namespace Lumos
