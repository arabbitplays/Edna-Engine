#include "../../include/engine/PresentStage.hpp"

#include <stdexcept>

namespace RtEngine {
    PresentStage::PresentStage(std::shared_ptr<VulkanContext> vulkan_context,
                               std::shared_ptr<SyncManager> sync_manager,
                               std::shared_ptr<GuiRenderer> gui_renderer,
                               const uint32_t max_frames_in_flight)
        : vulkan_context(std::move(vulkan_context)),
          sync_manager(std::move(sync_manager)),
          gui_renderer(std::move(gui_renderer)),
          max_frames_in_flight(max_frames_in_flight) {
    }

    void PresentStage::init() {
        command_buffers = vulkan_context->command_manager->allocatePrimaryCommandBuffers(max_frames_in_flight);
    }

    int32_t PresentStage::acquireNextSwapchainImage() {
        uint32_t image_index;
        VkResult result = vkAcquireNextImageKHR(vulkan_context->device_manager->getDevice(),
                                                vulkan_context->swapchain->handle,
                                                UINT64_MAX,
                                                sync_manager->imageAvailableSemaphore(),
                                                VK_NULL_HANDLE,
                                                &image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return -1;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("PresentStage: failed to acquire swap chain image");
        }
        return static_cast<int32_t>(image_index);
    }

    bool PresentStage::submitAndPresent(const uint32_t stage_index,
                                        const std::shared_ptr<ImageConnector> &source,
                                        const uint32_t swapchain_image_idx) {
        AllocatedImage source_image = source->getImageAt(currentFrameSlot());
        VkExtent2D source_extent = source->getExtent();

        VkCommandBuffer cmd = beginCommandBuffer();
        recordBlit(cmd, source_image, source_extent, swapchain_image_idx);
        transitionSwapchainForPresent(cmd, swapchain_image_idx);
        transitionSourceBackToGeneral(cmd, source_image);
        gui_renderer->recordGuiCommands(cmd, swapchain_image_idx);
        endCommandBuffer(cmd);

        sync_manager->submitStage(stage_index,
                                  vulkan_context->device_manager->getQueue(GRAPHICS),
                                  cmd,
                                  /*binary waits*/       {sync_manager->imageAvailableSemaphore()},
                                  /*binary wait stages*/ {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                                  /*binary signals*/     {sync_manager->renderFinishedSemaphore(swapchain_image_idx)});

        return present(swapchain_image_idx);
    }

    VkCommandBuffer PresentStage::beginCommandBuffer() {
        VkCommandBuffer cmd = command_buffers[currentFrameSlot()];
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("PresentStage: failed to begin command buffer");
        }
        return cmd;
    }

    void PresentStage::endCommandBuffer(VkCommandBuffer cmd) {
        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            throw std::runtime_error("PresentStage: failed to end command buffer");
        }
    }

    void PresentStage::recordBlit(VkCommandBuffer cmd,
                                  AllocatedImage source_image,
                                  VkExtent2D source_extent,
                                  const uint32_t swapchain_image_idx) {
        std::shared_ptr<ResourceBuilder> resource_builder = vulkan_context->resource_builder;
        std::shared_ptr<Swapchain> swapchain = vulkan_context->swapchain;

        resource_builder->transitionImageLayout(cmd, swapchain->images[swapchain_image_idx],
                                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // expects source to be in GENERAL
        // the timeline semaphoe handles execution ordering, this is only layout transition
        resource_builder->transitionImageLayout(cmd, source_image.image,
                                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                VK_ACCESS_NONE, VK_ACCESS_TRANSFER_READ_BIT,
                                                VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        const int32_t swapchain_width = swapchain->extent.width;
        const int32_t swapchain_height = swapchain->extent.height;
        const int32_t target_width = source_extent.width;
        const int32_t target_height = source_extent.height;

        VkImageBlit blit_region{};
        blit_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        blit_region.srcOffsets[0] = {0, 0, 0};
        blit_region.srcOffsets[1] = {target_width, target_height, 1};
        blit_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        blit_region.dstOffsets[0] = {0, 0, 0};
        blit_region.dstOffsets[1] = {swapchain_width, swapchain_height, 1};

        vkCmdBlitImage(cmd,
                       source_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       swapchain->images[swapchain_image_idx], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit_region, VK_FILTER_NEAREST);
    }

    void PresentStage::transitionSwapchainForPresent(VkCommandBuffer cmd, const uint32_t swapchain_image_idx) {
        // GUI render pass expects PRESENT_SRC_KHR as its initialLayout and preserves it
        vulkan_context->resource_builder->transitionImageLayout(
                cmd, vulkan_context->swapchain->images[swapchain_image_idx],
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    void PresentStage::transitionSourceBackToGeneral(VkCommandBuffer cmd, AllocatedImage source_image) {
        vulkan_context->resource_builder->transitionImageLayout(
                cmd, source_image.image,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_NONE,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
    }

    bool PresentStage::present(const uint32_t swapchain_image_idx) {
        VkSemaphore wait = sync_manager->renderFinishedSemaphore(swapchain_image_idx);

        VkPresentInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &wait;
        VkSwapchainKHR swapchains[] = {vulkan_context->swapchain->handle};
        info.swapchainCount = 1;
        info.pSwapchains = swapchains;
        info.pImageIndices = &swapchain_image_idx;

        VkResult result = vkQueuePresentKHR(vulkan_context->device_manager->getQueue(PRESENT), &info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            return true;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("PresentStage: failed to present swap chain image");
        }
        return false;
    }

    uint32_t PresentStage::currentFrameSlot() const {
        return sync_manager->currentFrameInFlight();
    }

    void PresentStage::cleanup() {
        deletion_queue.flush();
    }
} // RtEngine
