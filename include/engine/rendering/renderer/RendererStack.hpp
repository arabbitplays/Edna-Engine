#ifndef EDNA_ENGINE_RENDERERSTACK_HPP
#define EDNA_ENGINE_RENDERERSTACK_HPP
#include <engine/rendering/connectors/ImageConnector.hpp>
#include <engine/PresentStage.hpp>
#include <engine/rendering/renderer/Renderer.hpp>

#include <memory>
#include <vector>

namespace RtEngine
{
    class RendererStack
    {
    public:
        RendererStack() = default;

        void addRenderer(RendererHandle renderer);
        void setPresentStage(std::shared_ptr<PresentStage> present_stage);
        void setPresentConnector(std::shared_ptr<ImageConnector> connector);

        const std::vector<RendererHandle>& getRenderers() const;
        std::shared_ptr<PresentStage> getPresentStage() const;
        std::shared_ptr<ImageConnector> getPresentConnector() const;

    private:
        std::vector<RendererHandle> renderers;
        std::shared_ptr<PresentStage> present_stage;
        std::shared_ptr<ImageConnector> present_connector;
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_RENDERERSTACK_HPP
