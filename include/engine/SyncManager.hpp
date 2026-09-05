#ifndef EDNA_ENGINE_SYNCMANAGER_HPP
#define EDNA_ENGINE_SYNCMANAGER_HPP
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "DeletionQueue.hpp"
#include "DeviceManager.hpp"
#include "Swapchain.hpp"

namespace RtEngine
{
    class SyncManager
    {
    public:
        SyncManager(uint32_t max_frames_in_flight,
                    std::shared_ptr<DeviceManager> device_manager,
                    std::shared_ptr<Swapchain> swapchain);

        void setStagesPerFrame(uint32_t stages_per_frame);

        void waitForNextFrameStart();
        void advanceFrame();

        uint32_t currentFrameInFlight() const;

        // Timeline access for stage boundaries.
        VkSemaphore timeline() const;
        uint64_t stageWaitValue(uint32_t stage_index) const;
        uint64_t stageSignalValue(uint32_t stage_index) const;
        bool stageHasPredecessor(uint32_t stage_index) const;

        // Submit `command_buffer` as the given stage of the current frame.
        // Waits on the timeline at stageWaitValue(stage_index) when stage_index > 0,
        // and on any extra binary semaphores. Signals the timeline at
        // stageSignalValue(stage_index) and any extra binary semaphores.
        // The two extra_binary_waits vectors must be the same length.
        void submitStage(uint32_t stage_index,
                         VkQueue queue,
                         VkCommandBuffer command_buffer,
                         const std::vector<VkSemaphore> &extra_binary_waits = {},
                         const std::vector<VkPipelineStageFlags> &extra_binary_wait_stages = {},
                         const std::vector<VkSemaphore> &extra_binary_signals = {});

        // Host-signal the timeline to stageSignalValue(stage_index) without a GPU
        // submission. Use when a frame chooses not to run a stage but the frame-end
        // timeline value still needs to be reached so the CPU wait doesn't hang.
        void skipStage(uint32_t stage_index);

        // Swapchain binary semaphores.
        VkSemaphore imageAvailableSemaphore() const;
        VkSemaphore renderFinishedSemaphore(uint32_t swapchain_image_index) const;

        // Rebuild the per-swapchain-image render-finished semaphores after a swapchain resize.
        // Caller must have made sure the device is idle first.
        void recreateSwapchainSemaphores();

        void destroy();

    private:
        void createTimelineSemaphore();
        void createFrameSemaphores();
        void createSwapchainSemaphores();
        void destroySwapchainSemaphores();

        std::shared_ptr<DeviceManager> device_manager;
        std::shared_ptr<Swapchain> swapchain;

        uint32_t max_frames_in_flight;
        uint32_t stages_per_frame = 1;
        uint64_t frame_counter = 0;

        VkSemaphore timeline_semaphore = VK_NULL_HANDLE;
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkSemaphore> render_finished_semaphores;

        DeletionQueue deletion_queue;
    };
} // RtEngine

#endif //EDNA_ENGINE_SYNCMANAGER_HPP
