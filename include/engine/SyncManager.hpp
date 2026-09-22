#ifndef EDNA_ENGINE_SYNCMANAGER_HPP
#define EDNA_ENGINE_SYNCMANAGER_HPP
#include "DeletionQueue.hpp"
#include "DeviceManager.hpp"
#include "Swapchain.hpp"
#include "SwapchainManager.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace RtEngine
{
    class SyncManager
    {
    public:
        SyncManager(uint32_t max_frames_in_flight, std::shared_ptr<DeviceManager> device_manager,
            std::shared_ptr<SwapchainManager> swapchain_manager);

        // Caller must ensure the device is idle before calling.
        void reconfigureStagesPerFrame(uint32_t stages_per_frame);

        void waitForNextFrameStart();
        void advanceFrame();

        uint32_t currentFrameInFlight() const;

        // Invariant: within a single frame, every stage in [0, stages_per_frame) must be handled
        // exactly once via either submitStage or skipStage, in ascending stage_index order.
        // The timeline value advances by stages_per_frame per frame; waitForNextFrameStart relies
        // on this to compute when a frame slot is safe to reuse.
        void submitStage(uint32_t stage_index, VkQueue queue, VkCommandBuffer command_buffer,
            const std::vector<VkSemaphore>& extra_binary_waits = {},
            const std::vector<VkPipelineStageFlags>& extra_binary_wait_stages = {},
            const std::vector<VkSemaphore>& extra_binary_signals = {});

        // Signals the stage's timeline value without submitting any work. Must be called for any
        // stage that submitStage would otherwise cover this frame - see submitStage invariant.
        void skipStage(uint32_t stage_index);

        VkSemaphore imageAvailableSemaphore() const;
        VkSemaphore renderFinishedSemaphore(uint32_t swapchain_image_index) const;

        void destroy();

    private:
        VkSemaphore timeline() const;
        uint64_t stageWaitValue(uint32_t stage_index) const;
        uint64_t stageSignalValue(uint32_t stage_index) const;
        static bool stageHasPredecessor(uint32_t stage_index);

        void recreateSwapchainSemaphores();

        void createTimelineSemaphore();
        void createFrameSemaphores();
        void createSwapchainSemaphores();
        void destroySwapchainSemaphores();

        std::shared_ptr<DeviceManager> device_manager;
        std::shared_ptr<SwapchainManager> swapchain_manager;
        SwapchainManager::RecreateCallbackHandle resize_callback_handle = 0;

        uint32_t max_frames_in_flight;
        uint32_t stages_per_frame = 1;
        uint64_t frame_counter = 0;

        VkSemaphore timeline_semaphore = VK_NULL_HANDLE;
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkSemaphore> render_finished_semaphores;

        DeletionQueue deletion_queue;
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_SYNCMANAGER_HPP
