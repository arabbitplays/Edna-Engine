#ifndef VULKAN_RAYTRACING_RENDERER_HPP
#define VULKAN_RAYTRACING_RENDERER_HPP
#include "IRenderable.hpp"
#include "RenderTarget.hpp"
#include "SyncManager.hpp"
#include "VulkanContext.hpp"

namespace RtEngine {
    class Renderer {
    public:
        Renderer(std::shared_ptr<VulkanContext> vulkan_context,
                 std::shared_ptr<SyncManager> sync_manager,
                 uint32_t max_frames_in_flight);

        virtual void init();

        virtual void writeResources(const std::shared_ptr<DrawContext> &draw_context, UpdateFlagsHandle update_flags) = 0;
        virtual void writeRenderTarget(const std::shared_ptr<RenderTarget> &target) = 0;

        void waitForNextFrameStart();

        VkCommandBuffer getNextCommandBuffer();

        void nextFrame();

    protected:
        void createCommandBuffers();

        void submitStage(uint32_t stage_index,
                         VkQueue queue,
                         const std::vector<VkSemaphore> &extra_binary_waits = {},
                         const std::vector<VkPipelineStageFlags> &extra_binary_wait_stages = {},
                         const std::vector<VkSemaphore> &extra_binary_signals = {});

        uint32_t currentFrameSlot() const;

        std::shared_ptr<VulkanContext> vulkan_context;
        std::shared_ptr<SyncManager> sync_manager;
        uint32_t max_frames_in_flight;

        std::vector<VkCommandBuffer> command_buffers;

        DeletionQueue deletion_queue;
    };
} // RtEngine

#endif //VULKAN_RAYTRACING_RENDERER_HPP
