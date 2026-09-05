#include "../../../../include/engine/rendering/renderer/Renderer.hpp"

#include <stdexcept>

#include "MeshAsset.hpp"

namespace RtEngine {
    Renderer::Renderer(std::shared_ptr<VulkanContext> vulkan_context,
                       std::shared_ptr<SyncManager> sync_manager,
                       const uint32_t max_frames_in_flight)
        : vulkan_context(std::move(vulkan_context)),
          sync_manager(std::move(sync_manager)),
          max_frames_in_flight(max_frames_in_flight) {
    }

    void Renderer::init() {
        createCommandBuffers();
    }

    void Renderer::createCommandBuffers() {
        command_buffers.resize(max_frames_in_flight);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = vulkan_context->command_manager->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        if (vkAllocateCommandBuffers(vulkan_context->device_manager->getDevice(), &allocInfo, command_buffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command bufer!");
        }
    }

    void Renderer::waitForNextFrameStart() {
        sync_manager->waitForNextFrameStart();
    }

    VkCommandBuffer Renderer::getNewCommandBuffer() {
        VkCommandBuffer cmd = command_buffers[currentFrameSlot()];
        vkResetCommandBuffer(cmd, 0);
        return cmd;
    }

    void Renderer::nextFrame() {
        sync_manager->advanceFrame();
    }

    uint32_t Renderer::currentFrameSlot() const {
        return sync_manager->currentFrameInFlight();
    }

    void Renderer::submitStage(const uint32_t stage_index,
                               VkQueue queue,
                               const std::vector<VkSemaphore> &extra_binary_waits,
                               const std::vector<VkPipelineStageFlags> &extra_binary_wait_stages,
                               const std::vector<VkSemaphore> &extra_binary_signals) {
        sync_manager->submitStage(stage_index, queue, command_buffers[currentFrameSlot()],
                                  extra_binary_waits, extra_binary_wait_stages, extra_binary_signals);
    }
} // RtEngine
