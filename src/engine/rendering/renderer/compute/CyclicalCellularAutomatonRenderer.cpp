#include "compute/CyclicalCellularAutomatonRenderer.hpp"

#include <cassert>
#include <cyclical_cellular_automaton.comp.spv.h>
#include <random>

#include "BufferConnectorFactory.hpp"
#include "ImageConnectorFactory.hpp"
#include "VulkanUtil.hpp"

namespace RtEngine {
    CyclicalCellularAutomatonRenderer::CyclicalCellularAutomatonRenderer(
        const std::shared_ptr<VulkanContext>& vulkan_context,
        VkExtent2D image_extent,
        const std::vector<glm::vec4>& colors,
        const std::vector<glm::ivec2>& neighbor_offsets,
        const uint32_t max_frames_in_flight)
        : ComputeRenderer(vulkan_context, max_frames_in_flight),
          image_extent(image_extent), colors(colors) {
        assert(!colors.empty()
               && "CyclicalCellularAutomatonRenderer: colors must not be empty");
        assert(colors.size() <= MAX_STATE_COUNT
               && "CyclicalCellularAutomatonRenderer: colors.size() must not exceed MAX_STATE_COUNT (palette UBO capacity)");
        assert(!neighbor_offsets.empty()
               && "CyclicalCellularAutomatonRenderer: neighbor_offsets must not be empty");
        assert(neighbor_offsets.size() <= MAX_NEIGHBOR_COUNT
               && "CyclicalCellularAutomatonRenderer: neighbor_offsets.size() must not exceed MAX_NEIGHBOR_COUNT (neighborhood UBO capacity)");

        push.state_count = static_cast<uint32_t>(colors.size());

        state_connector = std::make_shared<ImageConnector>(
            vulkan_context->resource_builder, image_extent, 1,
            VK_FORMAT_R32_UINT,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        target_connector = ImageConnectorFactory::createRenderTargetConnector(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight);

        rng_connector = ImageConnectorFactory::createRngTextureConnector(
            vulkan_context->resource_builder, image_extent, 1);

        const VkDeviceSize palette_size = sizeof(glm::vec4) * MAX_STATE_COUNT;
        const VkDeviceSize upload_size = sizeof(glm::vec4) * colors.size();
        palette_connector = BufferConnectorFactory::createUniformBuffer(
            vulkan_context->resource_builder, vulkan_context->device_manager, palette_size, 1);
        palette_connector->uploadData(0, this->colors.data(), upload_size);

        neighborhood_connector = BufferConnectorFactory::createUniformBuffer(
            vulkan_context->resource_builder, vulkan_context->device_manager,
            neighborhoodBufferSize(), 1);
        setNeighborhood(neighbor_offsets);

        addConnector(0, state_connector);
        addConnector(1, target_connector);
        addConnector(2, palette_connector);
        addConnector(3, rng_connector);
        addConnector(4, neighborhood_connector);

        setDispatchSize([this]() {
            const VkExtent2D extent = target_connector->getExtent();
            return VkExtent3D{(extent.width + 15) / 16, (extent.height + 15) / 16, 1};
        });

        initializeState();

        deletion_queue.pushFunction([this]() {
            state_connector->destroy();
            target_connector->destroy();
            rng_connector->destroy();
            palette_connector->destroy();
            neighborhood_connector->destroy();
        });
    }

    void CyclicalCellularAutomatonRenderer::initializeState() {
        const uint32_t pixel_count = image_extent.width * image_extent.height;
        std::vector<uint32_t> noise(pixel_count);

        std::mt19937 rng(0xC0FFEE);
        std::uniform_int_distribution<uint32_t> dist(0, colors.size() - 1);
        for (uint32_t& v : noise) { v = dist(rng);
}

        const VkDeviceSize size = pixel_count * sizeof(uint32_t);
        const VkExtent3D extent_3d{image_extent.width, image_extent.height, 1};
        vulkan_context->resource_builder->uploadImageData(
            state_connector->getImageAt(0), extent_3d, noise.data(), size);
    }

    VkDeviceSize CyclicalCellularAutomatonRenderer::neighborhoodBufferSize() {
        // Matches std140 layout of the Neighborhood UBO in the shader:
        // ivec4 offsets[MAX_NEIGHBOR_COUNT] followed by a uint count padded to a vec4 slot.
        return (sizeof(glm::ivec4) * MAX_NEIGHBOR_COUNT) + sizeof(glm::ivec4);
    }

    void CyclicalCellularAutomatonRenderer::setNeighborhood(const std::vector<glm::ivec2>& new_offsets) {
        assert(!new_offsets.empty()
               && "CyclicalCellularAutomatonRenderer::setNeighborhood: offsets must not be empty");
        assert(new_offsets.size() <= MAX_NEIGHBOR_COUNT
               && "CyclicalCellularAutomatonRenderer::setNeighborhood: offsets.size() must not exceed MAX_NEIGHBOR_COUNT");

        std::vector<glm::ivec4> padded(new_offsets.size());
        for (size_t i = 0; i < new_offsets.size(); ++i) {
            padded[i] = glm::ivec4(new_offsets[i], 0, 0);
        }

        const VkDeviceSize offsets_size = sizeof(glm::ivec4) * padded.size();
        neighborhood_connector->uploadData(0, padded.data(), offsets_size);

        const auto count = static_cast<uint32_t>(new_offsets.size());
        const VkDeviceSize count_offset = sizeof(glm::ivec4) * MAX_NEIGHBOR_COUNT;
        neighborhood_connector->uploadData(0, &count, sizeof(count), count_offset);
    }

    void CyclicalCellularAutomatonRenderer::setPalette(const std::vector<glm::vec4>& new_colors) {
        assert(!new_colors.empty()
               && "CyclicalCellularAutomatonRenderer::setPalette: colors must not be empty");
        assert(new_colors.size() <= MAX_STATE_COUNT
               && "CyclicalCellularAutomatonRenderer::setPalette: colors.size() must not exceed MAX_STATE_COUNT");

        const bool state_count_changed = colors.size() != new_colors.size();
        colors = new_colors;
        push.state_count = static_cast<uint32_t>(colors.size());

        const VkDeviceSize upload_size = sizeof(glm::vec4) * colors.size();
        palette_connector->uploadData(0, colors.data(), upload_size);

        if (state_count_changed) {
            initializeState();
        }
    }

    void CyclicalCellularAutomatonRenderer::handleResize(VkExtent2D new_extent) {
        image_extent = new_extent;
        state_connector->recreate(new_extent);
        target_connector->recreate(new_extent);
        rng_connector->recreate(new_extent);
        initializeState();
        invalidateDescriptors();
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
