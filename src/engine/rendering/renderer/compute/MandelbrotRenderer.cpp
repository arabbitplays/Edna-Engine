#include "compute/MandelbrotRenderer.hpp"

#include <cassert>
#include <cmath>
#include <cstring>
#include <mandelbrot.comp.spv.h>
#include <vector>

#include "BufferConnectorFactory.hpp"
#include "ImageConnectorFactory.hpp"
#include "VulkanUtil.hpp"

namespace RtEngine {
    MandelbrotRenderer::MandelbrotRenderer(
        const std::shared_ptr<VulkanContext>& vulkan_context,
        VkExtent2D image_extent,
        const std::vector<glm::vec4>& colors,
        const double origin_x,
        const double origin_y,
        const double offset_x,
        const double offset_y,
        const double step_size,
        const uint32_t max_iterations,
        const double initial_x,
        const double initial_y,
        const bool julia_mode,
        const uint32_t max_frames_in_flight)
        : ComputeRenderer(vulkan_context, max_frames_in_flight),
          image_extent(image_extent), colors(colors) {
        assert(!colors.empty()
               && "MandelbrotRenderer: colors must not be empty");
        assert(colors.size() <= MAX_COLOR_COUNT
               && "MandelbrotRenderer: colors.size() must not exceed MAX_COLOR_COUNT (palette UBO capacity)");

        world_origin_x = origin_x;
        world_origin_y = origin_y;
        screen_offset_x = offset_x;
        screen_offset_y = offset_y;
        push.step_size = step_size;
        push.initial_x = initial_x;
        push.initial_y = initial_y;
        push.max_iterations = max_iterations;
        push.color_count = static_cast<uint32_t>(colors.size());
        push.julia_mode = julia_mode ? 1u : 0u;

        target_connector = ImageConnectorFactory::createRenderTargetConnector(
            vulkan_context->resource_builder, image_extent, max_frames_in_flight);

        const VkDeviceSize palette_size = sizeof(glm::vec4) * MAX_COLOR_COUNT;
        const VkDeviceSize upload_size = sizeof(glm::vec4) * colors.size();
        palette_connector = BufferConnectorFactory::createUniformBuffer(
            vulkan_context->resource_builder, vulkan_context->device_manager, palette_size, 1);
        palette_connector->uploadData(0, this->colors.data(), upload_size);

        const VkDeviceSize histogram_size = sizeof(uint32_t) * HISTOGRAM_BIN_COUNT;
        histogram_connector = std::make_shared<BufferConnector>(
            vulkan_context->resource_builder, vulkan_context->device_manager,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, histogram_size, 1,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        addConnector(0, target_connector);
        addConnector(1, palette_connector);
        addConnector(2, histogram_connector);

        setDispatchSize([this]() {
            const VkExtent2D extent = target_connector->getExtent();
            return VkExtent3D{(extent.width + 15) / 16, (extent.height + 15) / 16, 1};
        });

        deletion_queue.pushFunction([this]() {
            target_connector->destroy();
            palette_connector->destroy();
            histogram_connector->destroy();
        });
    }

    void MandelbrotRenderer::setPalette(const std::vector<glm::vec4>& new_colors) {
        assert(!new_colors.empty()
               && "MandelbrotRenderer::setPalette: colors must not be empty");
        assert(new_colors.size() <= MAX_COLOR_COUNT
               && "MandelbrotRenderer::setPalette: colors.size() must not exceed MAX_COLOR_COUNT");

        colors = new_colors;
        push.color_count = static_cast<uint32_t>(colors.size());

        const VkDeviceSize upload_size = sizeof(glm::vec4) * colors.size();
        palette_connector->uploadData(0, colors.data(), upload_size);
    }

    void MandelbrotRenderer::handleResize(VkExtent2D new_extent) {
        image_extent = new_extent;
        target_connector->recreate(new_extent);
        invalidateDescriptors();
    }

    std::shared_ptr<ImageConnector> MandelbrotRenderer::getOutputConnector() const {
        return target_connector;
    }

    VkShaderModule MandelbrotRenderer::createShaderModule() {
        return VulkanUtil::createShaderModule(
            vulkan_context->device_manager->getDevice(),
            oschd_mandelbrot_comp_spv_size(),
            oschd_mandelbrot_comp_spv());
    }

    void MandelbrotRenderer::configurePushConstants(ComputePipeline& pipeline) {
        pipeline.addPushConstant(sizeof(PushConstants), VK_SHADER_STAGE_COMPUTE_BIT);
    }

    void MandelbrotRenderer::recordPreDispatch(VkCommandBuffer cmd) {
        const VkDeviceSize histogram_size = sizeof(uint32_t) * HISTOGRAM_BIN_COUNT;
        vkCmdFillBuffer(cmd, histogram_connector->getBufferAt(0).handle, 0, histogram_size, 0u);

        VkBufferMemoryBarrier barrier{};
        barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer              = histogram_connector->getBufferAt(0).handle;
        barrier.offset              = 0;
        barrier.size                = histogram_size;

        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 0, nullptr, 1, &barrier, 0, nullptr);
    }

    float MandelbrotRenderer::readEntropy() const {
        const VkDeviceSize histogram_size = sizeof(uint32_t) * HISTOGRAM_BIN_COUNT;
        std::vector<uint32_t> bins(HISTOGRAM_BIN_COUNT, 0);

        const VkDevice device = vulkan_context->device_manager->getDevice();
        const AllocatedBuffer buffer = histogram_connector->getBufferAt(0);

        void* mapped = nullptr;
        vkMapMemory(device, buffer.bufferMemory, 0, histogram_size, 0, &mapped);
        std::memcpy(bins.data(), mapped, histogram_size);
        vkUnmapMemory(device, buffer.bufferMemory);

        uint64_t total = 0;
        for (const uint32_t c : bins) total += c;
        if (total == 0) return 0.0f;

        const double inv_total = 1.0 / static_cast<double>(total);
        double entropy = 0.0;
        for (const uint32_t c : bins) {
            if (c == 0u) continue;
            const double p = static_cast<double>(c) * inv_total;
            entropy -= p * std::log2(p);
        }
        return static_cast<float>(entropy);
    }

    void MandelbrotRenderer::recordPushConstants(VkCommandBuffer cmd) {
        const VkExtent2D extent = target_connector->getExtent();
        const double screen_width_c  = push.step_size * static_cast<double>(extent.width);
        const double screen_height_c = push.step_size * static_cast<double>(extent.height);
        push.origin_x = world_origin_x + screen_offset_x * screen_width_c  - 0.5 * screen_width_c;
        push.origin_y = world_origin_y + screen_offset_y * screen_height_c - 0.5 * screen_height_c;

        vkCmdPushConstants(cmd, pipeline->getLayoutHandle(),
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &push);
    }
} // RtEngine
