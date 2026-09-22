#include "compute/MandelbulbRenderer.hpp"

#include "BufferConnectorFactory.hpp"
#include "ImageConnectorFactory.hpp"
#include "VulkanUtil.hpp"

#include <cassert>
#include <cstring>
#include <mandelbulb.comp.spv.h>
#include <vector>

namespace RtEngine
{
    MandelbulbRenderer::MandelbulbRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
        const VkExtent2D image_extent, const std::vector<glm::vec4>& colors, const glm::vec3& initial,
        const float power, const uint32_t max_iterations, const ColoringMode coloring_mode,
        const uint32_t max_frames_in_flight)
        : ComputeRenderer(vulkan_context, max_frames_in_flight), image_extent(image_extent), colors(colors)
    {
        assert(!colors.empty() && "MandelbulbRenderer: colors must not be empty");
        assert(colors.size() <= MAX_COLOR_COUNT &&
               "MandelbulbRenderer: colors.size() must not exceed MAX_COLOR_COUNT (palette UBO capacity)");

        push.initial = glm::vec4(initial, 0.0f);
        push.power = power;
        push.max_iterations = max_iterations;
        push.color_count = static_cast<uint32_t>(colors.size());
        push.coloring_mode = static_cast<uint32_t>(coloring_mode);

        target_connector = ImageConnectorFactory::createRenderTargetConnector(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight);

        const VkDeviceSize palette_size = sizeof(glm::vec4) * MAX_COLOR_COUNT;
        const VkDeviceSize palette_upload_size = sizeof(glm::vec4) * colors.size();
        palette_connector = BufferConnectorFactory::createUniformBuffer(
            vulkan_context->resource_builder, vulkan_context->device_manager, palette_size, 1);
        palette_connector->uploadData(0, this->colors.data(), palette_upload_size);

        camera_connector = BufferConnectorFactory::createUniformBuffer(
            vulkan_context->resource_builder, vulkan_context->device_manager, sizeof(CameraData), 1);
        camera_connector->uploadData(0, &camera_data, sizeof(CameraData));

        addConnector(0, target_connector);
        addConnector(1, palette_connector);
        addConnector(2, camera_connector);

        setDispatchSize(
            [this]()
            {
                const VkExtent2D extent = target_connector->getExtent();
                return VkExtent3D{(extent.width + 15) / 16, (extent.height + 15) / 16, 1};
            });

        deletion_queue.pushFunction(
            [this]()
            {
                target_connector->destroy();
                palette_connector->destroy();
                camera_connector->destroy();
            });
    }

    void MandelbulbRenderer::setPalette(const std::vector<glm::vec4>& new_colors)
    {
        assert(!new_colors.empty() && "MandelbulbRenderer::setPalette: colors must not be empty");
        assert(new_colors.size() <= MAX_COLOR_COUNT &&
               "MandelbulbRenderer::setPalette: colors.size() must not exceed MAX_COLOR_COUNT");

        colors = new_colors;
        push.color_count = static_cast<uint32_t>(colors.size());

        const VkDeviceSize upload_size = sizeof(glm::vec4) * colors.size();
        palette_connector->uploadData(0, colors.data(), upload_size);
    }

    void MandelbulbRenderer::handleResize(VkExtent2D new_extent)
    {
        image_extent = new_extent;
        target_connector->recreate(new_extent);
        invalidateDescriptors();
    }

    std::shared_ptr<ImageConnector> MandelbulbRenderer::getOutputConnector() const
    {
        return target_connector;
    }

    VkShaderModule MandelbulbRenderer::createShaderModule()
    {
        return VulkanUtil::createShaderModule(
            vulkan_context->device_manager->getDevice(), oschd_mandelbulb_comp_spv_size(), oschd_mandelbulb_comp_spv());
    }

    void MandelbulbRenderer::configurePushConstants(ComputePipeline& pipeline)
    {
        pipeline.addPushConstant(sizeof(PushConstants), VK_SHADER_STAGE_COMPUTE_BIT);
    }

    void MandelbulbRenderer::recordPreDispatch(VkCommandBuffer /*cmd*/)
    {
        camera_connector->uploadData(0, &camera_data, sizeof(CameraData));
    }

    void MandelbulbRenderer::recordPushConstants(VkCommandBuffer cmd)
    {
        vkCmdPushConstants(
            cmd, pipeline->getLayoutHandle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &push);
    }
} // namespace RtEngine
