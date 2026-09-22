#include "../../../../include/engine/rendering/renderer/Renderer.hpp"

#include "MeshAsset.hpp"

#include <logging/LogManager.hpp>
#include <stdexcept>
#include <typeinfo>

namespace RtEngine
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<Renderer>();
            return instance;
        }
    } // namespace

    Renderer::Renderer(std::shared_ptr<VulkanContext> vulkan_context, const uint32_t max_frames_in_flight)
        : vulkan_context(std::move(vulkan_context)), max_frames_in_flight(max_frames_in_flight)
    {
    }

    void Renderer::setActive(bool value)
    {
        if (active == value)
        {
            return;
        }
        active = value;
        logger()->info(std::string(value ? "activated " : "deactivated ") + typeid(*this).name());
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

    void Renderer::recordBeginCommandBuffer(VkCommandBuffer& command_buffer)
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin record command buffer!");
        }
    }

    void Renderer::recordEndCommandBuffer(VkCommandBuffer& command_buffer)
    {
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

} // namespace RtEngine
