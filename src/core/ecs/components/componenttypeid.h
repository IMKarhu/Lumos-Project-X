#pragma once

namespace Lumos
{
    class ComponentTypeID
    {
    public:
        template<typename T>
        static size_t typeID() {
            static size_t typeID = m_nexttypeid++;
            return typeID;
        }
    private:
        static size_t m_nexttypeid;
    };

} //namespace Lumos
