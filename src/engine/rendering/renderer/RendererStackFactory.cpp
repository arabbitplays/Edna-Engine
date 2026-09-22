#include "RendererStackFactory.hpp"

#include "compute/GlitchRenderer.hpp"
#include "ImageConnectorFactory.hpp"

#include <utility>

namespace RtEngine
{
    RendererStackFactory::RendererStackFactory(std::shared_ptr<VulkanContext> vulkan_context,
        std::shared_ptr<PresentStage> present_stage, std::shared_ptr<MeshRepository> mesh_repository,
        std::shared_ptr<TextureRepository> texture_repository, const uint32_t max_frames_in_flight)
        : vulkan_context(std::move(vulkan_context)), present_stage(std::move(present_stage)),
          mesh_repository(std::move(mesh_repository)), texture_repository(std::move(texture_repository)),
          max_frames_in_flight(max_frames_in_flight)
    {
    }

    std::shared_ptr<RendererStack> RendererStackFactory::makeStack() const
    {
        auto stack = std::make_shared<RendererStack>();
        stack->setPresentStage(present_stage);
        return stack;
    }

    RendererStackFactory::Result RendererStackFactory::createEmptyStack()
    {
        return {.stack = makeStack(),
            /*raytracing_renderer*/ .raytracing_renderer = nullptr,
            /*raytracing_target_connector*/ .raytracing_target_connector = nullptr};
    }

    RendererStackFactory::Result RendererStackFactory::createRaytracingStack()
    {
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        auto rt_target_connector = ImageConnectorFactory::createRenderTargetConnector(
            vulkan_context->resource_builder, extent, max_frames_in_flight);

        auto glitch_renderer =
            std::make_shared<GlitchRenderer>(vulkan_context, extent, rt_target_connector, max_frames_in_flight);
        glitch_renderer->init();

        auto raytracing_renderer = std::make_shared<RaytracingRenderer>(
            vulkan_context, mesh_repository, texture_repository, max_frames_in_flight);
        raytracing_renderer->init();

        auto stack = makeStack();
        stack->addRenderer(raytracing_renderer);
        stack->addRenderer(glitch_renderer);
        stack->setPresentConnector(glitch_renderer->getOutputConnector());

        return {.stack = stack,
            .raytracing_renderer = raytracing_renderer,
            .raytracing_target_connector = rt_target_connector};
    }
} // namespace RtEngine
