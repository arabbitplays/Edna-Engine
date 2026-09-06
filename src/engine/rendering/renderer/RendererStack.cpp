#include "RendererStack.hpp"

#include <utility>

namespace RtEngine
{
    void RendererStack::addRenderer(RendererHandle renderer)
    {
        renderers.push_back(std::move(renderer));
    }

    void RendererStack::setPresentStage(std::shared_ptr<PresentStage> present_stage)
    {
        this->present_stage = std::move(present_stage);
    }

    void RendererStack::setPresentConnector(std::shared_ptr<ImageConnector> connector)
    {
        present_connector = std::move(connector);
    }

    const std::vector<RendererHandle>& RendererStack::getRenderers() const
    {
        return renderers;
    }

    std::shared_ptr<PresentStage> RendererStack::getPresentStage() const
    {
        return present_stage;
    }

    std::shared_ptr<ImageConnector> RendererStack::getPresentConnector() const
    {
        return present_connector;
    }
} // RtEngine
