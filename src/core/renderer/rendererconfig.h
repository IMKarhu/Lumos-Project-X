#pragma once
#include <string>

namespace Lumos
{
    struct RendererConfig
    {
        std::string name = "";
        uint32_t framesinflight = 2;
        uint32_t internalwidth = 640;
        uint32_t internalheight = 320;
        bool validationlayers = true;
        bool vsync = true;
    };
} //namespace Lumos
