#include "compute/GlitchRenderer.hpp"

#include <glitch.comp.spv.h>

namespace RtEngine {
    GlitchRenderer::GlitchRenderer(const std::shared_ptr<VulkanContext> &vulkan_context,
                                   VkExtent2D image_extent,
                                   std::shared_ptr<ImageConnector> input_connector,
                                   const uint32_t max_frames_in_flight) : ComputeRenderer(
        vulkan_context, max_frames_in_flight) {
        output_connector = std::make_shared<ImageConnector>(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight,
            VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        addConnector(0, output_connector);
        addConnector(1, std::move(input_connector));

        setDispatchSize([this]() {
            VkExtent2D extent = output_connector->getExtent();
            return VkExtent3D{1, (extent.height + 255) / 256, 1};
        });

        deletion_queue.pushFunction([this]() { output_connector->destroy(); });
    }

    std::shared_ptr<ImageConnector> GlitchRenderer::getOutputConnector() const {
        return output_connector;
    }

    VkShaderModule GlitchRenderer::createShaderModule() {
        return VulkanUtil::createShaderModule(
            vulkan_context->device_manager->getDevice(), oschd_glitch_comp_spv_size(), oschd_glitch_comp_spv());
    }

}
