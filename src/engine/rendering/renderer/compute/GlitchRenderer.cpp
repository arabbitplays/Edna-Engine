#include "compute/GlitchRenderer.hpp"

#include "ImageConnectorFactory.hpp"
#include "VulkanUtil.hpp"

#include <glitch.comp.spv.h>

namespace RtEngine
{
    GlitchRenderer::GlitchRenderer(const std::shared_ptr<VulkanContext>& vulkan_context, VkExtent2D image_extent,
        std::shared_ptr<ImageConnector> input_connector, const uint32_t max_frames_in_flight)
        : ComputeRenderer(vulkan_context, max_frames_in_flight)
    {
        output_connector = ImageConnectorFactory::createRenderTargetConnector(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight);

        addConnector(0, output_connector);
        addConnector(1, std::move(input_connector));

        setDispatchSize(
            [this]()
            {
                VkExtent2D extent = output_connector->getExtent();
                return VkExtent3D{(extent.width + 15) / 16, (extent.height + 15) / 16, 1};
            });

        start_time = std::chrono::steady_clock::now();

        deletion_queue.pushFunction([this]() { output_connector->destroy(); });
    }

    std::shared_ptr<ImageConnector> GlitchRenderer::getOutputConnector() const
    {
        return output_connector;
    }

    void GlitchRenderer::handleResize(VkExtent2D new_extent)
    {
        output_connector->recreate(new_extent);
        invalidateDescriptors();
    }

    VkShaderModule GlitchRenderer::createShaderModule()
    {
        return VulkanUtil::createShaderModule(
            vulkan_context->device_manager->getDevice(), oschd_glitch_comp_spv_size(), oschd_glitch_comp_spv());
    }

    void GlitchRenderer::configurePushConstants(ComputePipeline& pipeline)
    {
        pipeline.addPushConstant(sizeof(PushConstants), VK_SHADER_STAGE_COMPUTE_BIT);
    }

    void GlitchRenderer::recordPushConstants(VkCommandBuffer cmd)
    {
        const auto now = std::chrono::steady_clock::now();
        push.time = std::chrono::duration<float>(now - start_time).count();

        vkCmdPushConstants(
            cmd, pipeline->getLayoutHandle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &push);
    }

} // namespace RtEngine
