#pragma once
#include <unordered_set>
#include "windowing/events.h"

class InputSystem
{
public:
    InputSystem();
private:
    Lumos::EventBus m_eventbus;
    Lumos::EventQueue m_eventqueue;
    std::unordered_set<int> m_heldkeys;
};
