#pragma once
#include "device.h"
#include "spriterenderer.h"
#include "scenerenderer.h"
#include <memory>

namespace Lumos
{
    class RendererBase;
    class RenderFactory
    {
    public:
        RenderFactory() = default;
        RenderFactory(RenderFactory&& other) noexcept = default;
        RenderFactory& operator=(RenderFactory&& other) noexcept = default;
        RenderFactory(const RenderFactory&) = delete;
        RenderFactory& operator=(const RenderFactory&) = delete;
        ~RenderFactory() = default;
        std::unique_ptr<RendererBase> createSpriteRenderer(Device& device, SpriteRenderer::RendererConstructDescription rcd) const
        {
            return std::make_unique<SpriteRenderer>(device, rcd);
        }

        std::unique_ptr<RendererBase> createSceneRenderer(Device& device, SceneRenderer::RendererConstructDescription rcd)
        {
            return std::make_unique<SceneRenderer>(device, rcd);
        }
    };

} //namespace Lumos
