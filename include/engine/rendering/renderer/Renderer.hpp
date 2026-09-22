#ifndef VULKAN_RAYTRACING_RENDERER_HPP
#define VULKAN_RAYTRACING_RENDERER_HPP
#include "DeviceManager.hpp"
#include "IRenderable.hpp"
#include "RenderTarget.hpp"
#include "VulkanContext.hpp"

namespace RtEngine
{
    class Renderer
    {
    public:
        Renderer(std::shared_ptr<VulkanContext> vulkan_context, uint32_t max_frames_in_flight);

        virtual ~Renderer() = default;

        virtual void init();
        virtual void cleanup();

        // Called when a bound connector's underlying resources are recreated (e.g. an
        // ImageConnector is resized) so subclasses can re-write their descriptor sets.
        // No-op by default.
        virtual void invalidateDescriptors()
        {
        }

        virtual VkCommandBuffer recordCommandBuffer(uint32_t frame_idx) = 0;

        virtual QueueType queueType() const = 0;

    protected:
        VkCommandBuffer getFreshCommandBuffer(uint32_t frame_idx);
        static void recordBeginCommandBuffer(VkCommandBuffer& commandBuffer);
        static void recordEndCommandBuffer(VkCommandBuffer& commandBuffer);

        std::shared_ptr<VulkanContext> vulkan_context;
        uint32_t max_frames_in_flight;
        DeletionQueue deletion_queue;

    private:
        void createCommandBuffers();

        std::vector<VkCommandBuffer> command_buffers;
    };

    using RendererHandle = std::shared_ptr<Renderer>;
} // namespace RtEngine

#endif // VULKAN_RAYTRACING_RENDERER_HPP
