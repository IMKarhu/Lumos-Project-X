#include "inputsystem.h"

InputSystem::InputSystem()
{
    m_eventbus.subscribe<Lumos::ScrollEvent>([this](const Lumos::ScrollEvent& e) {
            std::println("scrolling event x: {}  y: {}", e.xoffset, e.yoffset);
            });
}
