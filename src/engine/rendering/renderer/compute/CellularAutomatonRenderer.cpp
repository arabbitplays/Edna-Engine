#include "compute/CellularAutomatonRenderer.hpp"

#include <cassert>
#include <cellular_automaton.comp.spv.h>

#include "VulkanUtil.hpp"

namespace RtEngine {
    CellularAutomatonRenderer::CellularAutomatonRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                                                         VkExtent2D image_extent,
                                                         std::vector<glm::vec4> colors,
                                                         const uint32_t max_frames_in_flight)
        : ComputeRenderer(vulkan_context, max_frames_in_flight),
          colors(std::move(colors)) {
        assert(!this->colors.empty() && "CellularAutomatonRenderer: color list must not be empty");

        output_connector = std::make_shared<ImageConnector>(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight,
            VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        addConnector(0, output_connector);

        setDispatchSize([this]() {
            VkExtent2D extent = output_connector->getExtent();
            return VkExtent3D{(extent.width + 15) / 16, (extent.height + 15) / 16, 1};
        });

        deletion_queue.pushFunction([this]() { output_connector->destroy(); });
    }

    std::shared_ptr<ImageConnector> CellularAutomatonRenderer::getOutputConnector() const {
        return output_connector;
    }

    VkShaderModule CellularAutomatonRenderer::createShaderModule() {
        return VulkanUtil::createShaderModule(
            vulkan_context->device_manager->getDevice(),
            oschd_cellular_automaton_comp_spv_size(),
            oschd_cellular_automaton_comp_spv());
    }

    void CellularAutomatonRenderer::configurePushConstants(ComputePipeline& pipeline) {
        pipeline.addPushConstant(sizeof(glm::vec4), VK_SHADER_STAGE_COMPUTE_BIT);
    }

    void CellularAutomatonRenderer::recordPushConstants(VkCommandBuffer cmd) {
        const glm::vec4 color = colors[color_index];
        color_index = (color_index + 1) % colors.size();

        vkCmdPushConstants(cmd, pipeline->getLayoutHandle(),
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(glm::vec4), &color);
    }
} // RtEngine
