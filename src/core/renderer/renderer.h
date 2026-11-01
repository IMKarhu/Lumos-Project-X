#pragma once
#include "camera.h"
#include <glm/glm.hpp>

#ifdef _WIN32
    #ifdef CORE_EXPORT
    #define CORE_API __declspec(dllexport)
    #else
    #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API
#endif


namespace Lumos
{
    class CORE_API Renderer
    {
    public:
        Renderer();
        ~Renderer();
        void update(float dt);
    private:
        Camera2D m_camera;
        void uniformBuffers();
        void updateUBOs(float dt, uint32_t currentimage);
    };


} //namespace Lumos
