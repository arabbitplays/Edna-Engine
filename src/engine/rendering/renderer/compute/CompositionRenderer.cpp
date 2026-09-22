#include "compute/CompositionRenderer.hpp"

#include "ImageConnectorFactory.hpp"
#include "VulkanUtil.hpp"

#include <composition.comp.spv.h>

namespace RtEngine
{
    CompositionRenderer::CompositionRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
        VkExtent2D image_extent, std::shared_ptr<ImageConnector> input_a, std::shared_ptr<ImageConnector> input_b,
        const uint32_t max_frames_in_flight)
        : ComputeRenderer(vulkan_context, max_frames_in_flight)
    {
        output_connector = ImageConnectorFactory::createRenderTargetConnector(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight);

        addConnector(0, output_connector);
        addConnector(1, std::move(input_a));
        addConnector(2, std::move(input_b));

        setDispatchSize(
            [this]()
            {
                const VkExtent2D extent = output_connector->getExtent();
                return VkExtent3D{(extent.width + 15) / 16, (extent.height + 15) / 16, 1};
            });

        deletion_queue.pushFunction([this]() { output_connector->destroy(); });
    }

    std::shared_ptr<ImageConnector> CompositionRenderer::getOutputConnector() const
    {
        return output_connector;
    }

    void CompositionRenderer::handleResize(VkExtent2D new_extent)
    {
        output_connector->recreate(new_extent);
        invalidateDescriptors();
    }

    VkShaderModule CompositionRenderer::createShaderModule()
    {
        return VulkanUtil::createShaderModule(vulkan_context->device_manager->getDevice(),
            oschd_composition_comp_spv_size(), oschd_composition_comp_spv());
    }

    void CompositionRenderer::configurePushConstants(ComputePipeline& pipeline)
    {
        pipeline.addPushConstant(sizeof(PushConstants), VK_SHADER_STAGE_COMPUTE_BIT);
    }

    void CompositionRenderer::recordPushConstants(VkCommandBuffer cmd)
    {
        vkCmdPushConstants(
            cmd, pipeline->getLayoutHandle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &push);
    }
} // namespace RtEngine
