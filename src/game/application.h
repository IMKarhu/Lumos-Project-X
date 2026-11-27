#pragma once
#include "engine/engine.h"
#include "ecs/ecs.h"
#include "inputsystem.h"

class Application
{
public:
    Application();
    void initEngine();
    void run();
private:
    Lumos::LumosEngine m_engine;
    Ecs m_ecs;
    InputSystem m_inputsystem{m_ecs};
};
