#include "../../../../include/engine/rendering/renderer/Renderer.hpp"

#include <stdexcept>

#include "MeshAsset.hpp"

namespace RtEngine
{
    Renderer::Renderer(std::shared_ptr<VulkanContext> vulkan_context,
                       const uint32_t max_frames_in_flight)
        : vulkan_context(std::move(vulkan_context)),
          max_frames_in_flight(max_frames_in_flight)
    {
    }

    void Renderer::init()
    {
        createCommandBuffers();
    }

    void Renderer::cleanup()
    {
        deletion_queue.flush();
    }

    void Renderer::createCommandBuffers()
    {
        command_buffers = vulkan_context->command_manager->allocatePrimaryCommandBuffers(max_frames_in_flight);
    }

    VkCommandBuffer Renderer::getFreshCommandBuffer(uint32_t frame_idx)
    {
        assert(frame_idx < command_buffers.size());
        VkCommandBuffer cmd = command_buffers[frame_idx];
        vkResetCommandBuffer(cmd, 0);
        return cmd;
    }

    void Renderer::recordBeginCommandBuffer(VkCommandBuffer& commandBuffer) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin record command buffer!");
        }
    }

    void Renderer::recordEndCommandBuffer(VkCommandBuffer& commandBuffer) {
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

}// RtEngine
