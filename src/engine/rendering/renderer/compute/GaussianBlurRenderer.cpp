#include "compute/GaussianBlurRenderer.hpp"

#include "ImageConnectorFactory.hpp"
#include "VulkanUtil.hpp"

#include <algorithm>
#include <gaussian_blur.comp.spv.h>

namespace RtEngine
{
    namespace
    {
        // Kernel of ±radius should cover ±3σ so <1% of the weight is
        // truncated; the 0.5 floor keeps sigma positive when radius is
        // very small.
        float sigmaForRadius(uint32_t radius)
        {
            return std::max(0.5F, static_cast<float>(radius) / 3.0F);
        }
    } // namespace

    GaussianBlurRenderer::GaussianBlurRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
        VkExtent2D image_extent, std::shared_ptr<ImageConnector> input_connector, const Direction direction,
        const uint32_t max_frames_in_flight)
        : ComputeRenderer(vulkan_context, max_frames_in_flight)
    {
        push.direction_x = (direction == Direction::Horizontal) ? 1 : 0;
        push.direction_y = (direction == Direction::Vertical) ? 1 : 0;
        push.radius = 0u;
        push.sigma = sigmaForRadius(push.radius);

        output_connector = ImageConnectorFactory::createRenderTargetConnector(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight);

        addConnector(0, output_connector);
        addConnector(1, std::move(input_connector));

        setDispatchSize(
            [this]()
            {
                const VkExtent2D extent = output_connector->getExtent();
                return VkExtent3D{(extent.width + 15) / 16, (extent.height + 15) / 16, 1};
            });

        deletion_queue.pushFunction([this]() { output_connector->destroy(); });
    }

    std::shared_ptr<ImageConnector> GaussianBlurRenderer::getOutputConnector() const
    {
        return output_connector;
    }

    void GaussianBlurRenderer::setRadius(const uint32_t radius)
    {
        const uint32_t clamped = std::min(radius, MAX_RADIUS);
        push.radius = clamped;
        push.sigma = sigmaForRadius(clamped);
    }

    void GaussianBlurRenderer::handleResize(VkExtent2D new_extent)
    {
        output_connector->recreate(new_extent);
        invalidateDescriptors();
    }

    VkShaderModule GaussianBlurRenderer::createShaderModule()
    {
        return VulkanUtil::createShaderModule(vulkan_context->device_manager->getDevice(),
            oschd_gaussian_blur_comp_spv_size(), oschd_gaussian_blur_comp_spv());
    }

    void GaussianBlurRenderer::configurePushConstants(ComputePipeline& pipeline)
    {
        pipeline.addPushConstant(sizeof(PushConstants), VK_SHADER_STAGE_COMPUTE_BIT);
    }

    void GaussianBlurRenderer::recordPushConstants(VkCommandBuffer cmd)
    {
        vkCmdPushConstants(
            cmd, pipeline->getLayoutHandle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &push);
    }
} // namespace RtEngine
