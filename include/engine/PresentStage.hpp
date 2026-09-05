#ifndef EDNA_ENGINE_PRESENTSTAGE_HPP
#define EDNA_ENGINE_PRESENTSTAGE_HPP
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "DeletionQueue.hpp"
#include "GuiRenderer.hpp"
#include "RenderTarget.hpp"
#include "SyncManager.hpp"
#include "VulkanContext.hpp"

namespace RtEngine {
    class PresentStage {
    public:
        PresentStage(std::shared_ptr<VulkanContext> vulkan_context,
                     std::shared_ptr<SyncManager> sync_manager,
                     std::shared_ptr<GuiRenderer> gui_renderer,
                     uint32_t max_frames_in_flight);

        void init();

        int32_t acquireNextSwapchainImage();

        bool submitAndPresent(uint32_t stage_index,
                              const std::shared_ptr<RenderTarget> &source,
                              uint32_t swapchain_image_idx);

        void cleanup();

    private:
        void createCommandBuffers();
        VkCommandBuffer beginCommandBuffer();
        void endCommandBuffer(VkCommandBuffer cmd);

        void recordBlit(VkCommandBuffer cmd,
                        const std::shared_ptr<RenderTarget> &source,
                        uint32_t swapchain_image_idx);
        void transitionSwapchainForPresent(VkCommandBuffer cmd, uint32_t swapchain_image_idx);
        void transitionSourceBackToGeneral(VkCommandBuffer cmd, const std::shared_ptr<RenderTarget> &source);

        bool present(uint32_t swapchain_image_idx);

        uint32_t currentFrameSlot() const;

        std::shared_ptr<VulkanContext> vulkan_context;
        std::shared_ptr<SyncManager> sync_manager;
        std::shared_ptr<GuiRenderer> gui_renderer;
        uint32_t max_frames_in_flight;

        std::vector<VkCommandBuffer> command_buffers;
        DeletionQueue deletion_queue;
    };
} // RtEngine

#endif //EDNA_ENGINE_PRESENTSTAGE_HPP
