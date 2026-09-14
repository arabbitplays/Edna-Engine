#include "compute/CyclicalCellularAutomatonRenderer.hpp"

#include <cyclical_cellular_automaton.comp.spv.h>
#include <random>

#include "VulkanUtil.hpp"

namespace RtEngine {
    CyclicalCellularAutomatonRenderer::CyclicalCellularAutomatonRenderer(
        const std::shared_ptr<VulkanContext>& vulkan_context,
        VkExtent2D image_extent,
        const std::vector<glm::vec4>& colors,
        const uint32_t max_frames_in_flight)
        : ComputeRenderer(vulkan_context, max_frames_in_flight),
          image_extent(image_extent), colors(colors) {

        state_connector = std::make_shared<ImageConnector>(
            vulkan_context->resource_builder, image_extent, 1,
            VK_FORMAT_R32_UINT,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        target_connector = std::make_shared<ImageConnector>(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        const VkDeviceSize palette_size = sizeof(glm::vec4) * colors.size();
        palette_connector = std::make_shared<BufferConnector>(
            vulkan_context->resource_builder, vulkan_context->device_manager,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, palette_size, 1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        palette_connector->uploadData(0, this->colors.data(), palette_size);

        addConnector(0, state_connector);
        addConnector(1, target_connector);
        addConnector(2, palette_connector);

        setDispatchSize([this]() {
            const VkExtent2D extent = target_connector->getExtent();
            return VkExtent3D{(extent.width + 15) / 16, (extent.height + 15) / 16, 1};
        });

        initializeState();

        deletion_queue.pushFunction([this]() {
            state_connector->destroy();
            target_connector->destroy();
            palette_connector->destroy();
        });
    }

    void CyclicalCellularAutomatonRenderer::initializeState() {
        const uint32_t pixel_count = image_extent.width * image_extent.height;
        std::vector<uint32_t> noise(pixel_count);

        std::mt19937 rng(0xC0FFEE);
        std::uniform_int_distribution<uint32_t> dist(0, colors.size() - 1);
        for (uint32_t& v : noise) v = dist(rng);

        const VkDeviceSize size = pixel_count * sizeof(uint32_t);
        const VkExtent3D extent_3d{image_extent.width, image_extent.height, 1};
        vulkan_context->resource_builder->uploadImageData(
            state_connector->getImageAt(0), extent_3d, noise.data(), size);
    }

    void CyclicalCellularAutomatonRenderer::handleResize(VkExtent2D new_extent) {
        image_extent = new_extent;
        state_connector->recreate(new_extent);
        target_connector->recreate(new_extent);
        initializeState();
    }

    std::shared_ptr<ImageConnector> CyclicalCellularAutomatonRenderer::getOutputConnector() const {
        return target_connector;
    }

    VkShaderModule CyclicalCellularAutomatonRenderer::createShaderModule() {
        return VulkanUtil::createShaderModule(
            vulkan_context->device_manager->getDevice(),
            oschd_cyclical_cellular_automaton_comp_spv_size(),
            oschd_cyclical_cellular_automaton_comp_spv());
    }

    void CyclicalCellularAutomatonRenderer::configurePushConstants(ComputePipeline& pipeline) {
        pipeline.addPushConstant(sizeof(PushConstants), VK_SHADER_STAGE_COMPUTE_BIT);
    }

    void CyclicalCellularAutomatonRenderer::recordPushConstants(VkCommandBuffer cmd) {
        vkCmdPushConstants(cmd, pipeline->getLayoutHandle(),
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &push);
    }
} // RtEngine
