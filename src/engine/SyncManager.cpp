#include "../../include/engine/SyncManager.hpp"

#include <stdexcept>

namespace RtEngine
{
    SyncManager::SyncManager(uint32_t max_frames_in_flight,
                             std::shared_ptr<DeviceManager> device_manager,
                             std::shared_ptr<Swapchain> swapchain)
        : device_manager(std::move(device_manager)),
          swapchain(std::move(swapchain)),
          max_frames_in_flight(max_frames_in_flight) {
        createTimelineSemaphore();
        createFrameSemaphores();
        createSwapchainSemaphores();
    }

    void SyncManager::setStagesPerFrame(uint32_t stages_per_frame) {
        if (stages_per_frame == 0) {
            throw std::runtime_error("SyncManager: stages_per_frame must be >= 1");
        }
        this->stages_per_frame = stages_per_frame;
    }

    void SyncManager::waitForNextFrameStart() {
        if (frame_counter + 1 <= max_frames_in_flight) {
            return;
        }
        const uint64_t needed = (frame_counter + 1 - max_frames_in_flight) * stages_per_frame;

        VkSemaphoreWaitInfo wait_info{};
        wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &timeline_semaphore;
        wait_info.pValues = &needed;

        if (vkWaitSemaphores(device_manager->getDevice(), &wait_info, UINT64_MAX) != VK_SUCCESS) {
            throw std::runtime_error("SyncManager: vkWaitSemaphores failed");
        }
    }

    void SyncManager::advanceFrame() {
        frame_counter++;
    }

    uint32_t SyncManager::currentFrameInFlight() const {
        return static_cast<uint32_t>(frame_counter % max_frames_in_flight);
    }

    VkSemaphore SyncManager::timeline() const {
        return timeline_semaphore;
    }

    uint64_t SyncManager::stageWaitValue(uint32_t stage_index) const {
        return frame_counter * stages_per_frame + stage_index;
    }

    uint64_t SyncManager::stageSignalValue(uint32_t stage_index) const {
        return frame_counter * stages_per_frame + stage_index + 1;
    }

    bool SyncManager::stageHasPredecessor(uint32_t stage_index) const {
        return stage_index > 0;
    }

    void SyncManager::submitStage(const uint32_t stage_index,
                                  VkQueue queue,
                                  VkCommandBuffer command_buffer,
                                  const std::vector<VkSemaphore> &extra_binary_waits,
                                  const std::vector<VkPipelineStageFlags> &extra_binary_wait_stages,
                                  const std::vector<VkSemaphore> &extra_binary_signals) {
        if (extra_binary_waits.size() != extra_binary_wait_stages.size()) {
            throw std::runtime_error("SyncManager::submitStage: extra binary wait / stage size mismatch");
        }

        std::vector<VkSemaphore> wait_semaphores;
        std::vector<VkPipelineStageFlags> wait_stages;
        std::vector<uint64_t> wait_values;

        if (stageHasPredecessor(stage_index)) {
            wait_semaphores.push_back(timeline_semaphore);
            wait_values.push_back(stageWaitValue(stage_index));
            wait_stages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        }

        for (size_t i = 0; i < extra_binary_waits.size(); i++) {
            wait_semaphores.push_back(extra_binary_waits[i]);
            wait_values.push_back(0); // ignored for binary semaphores
            wait_stages.push_back(extra_binary_wait_stages[i]);
        }

        std::vector<VkSemaphore> signal_semaphores;
        std::vector<uint64_t> signal_values;

        signal_semaphores.push_back(timeline_semaphore);
        signal_values.push_back(stageSignalValue(stage_index));

        for (VkSemaphore s : extra_binary_signals) {
            signal_semaphores.push_back(s);
            signal_values.push_back(0); // ignored for binary semaphores
        }

        VkTimelineSemaphoreSubmitInfo timeline_info{};
        timeline_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timeline_info.waitSemaphoreValueCount = static_cast<uint32_t>(wait_values.size());
        timeline_info.pWaitSemaphoreValues = wait_values.data();
        timeline_info.signalSemaphoreValueCount = static_cast<uint32_t>(signal_values.size());
        timeline_info.pSignalSemaphoreValues = signal_values.data();

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = &timeline_info;
        submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
        submit_info.pWaitSemaphores = wait_semaphores.data();
        submit_info.pWaitDstStageMask = wait_stages.data();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
        submit_info.pSignalSemaphores = signal_semaphores.data();

        if (vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
            throw std::runtime_error("SyncManager::submitStage: vkQueueSubmit failed");
        }
    }

    void SyncManager::skipStage(const uint32_t stage_index) {
        VkSemaphoreSignalInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        info.semaphore = timeline_semaphore;
        info.value = stageSignalValue(stage_index);

        if (vkSignalSemaphore(device_manager->getDevice(), &info) != VK_SUCCESS) {
            throw std::runtime_error("SyncManager::skipStage: vkSignalSemaphore failed");
        }
    }

    VkSemaphore SyncManager::imageAvailableSemaphore() const {
        return image_available_semaphores[currentFrameInFlight()];
    }

    VkSemaphore SyncManager::renderFinishedSemaphore(uint32_t swapchain_image_index) const {
        return render_finished_semaphores[swapchain_image_index];
    }

    void SyncManager::recreateSwapchainSemaphores() {
        destroySwapchainSemaphores();
        createSwapchainSemaphores();
    }

    void SyncManager::destroy() {
        deletion_queue.flush();
        destroySwapchainSemaphores();
    }

    void SyncManager::createTimelineSemaphore() {
        VkSemaphoreTypeCreateInfo type_info{};
        type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        type_info.initialValue = 0;

        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = &type_info;

        if (vkCreateSemaphore(device_manager->getDevice(), &info, nullptr, &timeline_semaphore) != VK_SUCCESS) {
            throw std::runtime_error("SyncManager: failed to create timeline semaphore");
        }

        deletion_queue.pushFunction([this]() {
            vkDestroySemaphore(device_manager->getDevice(), timeline_semaphore, nullptr);
        });
    }

    void SyncManager::createFrameSemaphores() {
        image_available_semaphores.resize(max_frames_in_flight);

        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (uint32_t i = 0; i < max_frames_in_flight; i++) {
            if (vkCreateSemaphore(device_manager->getDevice(), &info, nullptr, &image_available_semaphores[i]) !=
                VK_SUCCESS) {
                throw std::runtime_error("SyncManager: failed to create imageAvailable semaphore");
            }

            deletion_queue.pushFunction([this, i]() {
                vkDestroySemaphore(device_manager->getDevice(), image_available_semaphores[i], nullptr);
            });
        }
    }

    void SyncManager::createSwapchainSemaphores() {
        const uint32_t count = static_cast<uint32_t>(swapchain->images.size());
        render_finished_semaphores.resize(count);

        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (uint32_t i = 0; i < count; i++) {
            if (vkCreateSemaphore(device_manager->getDevice(), &info, nullptr, &render_finished_semaphores[i]) !=
                VK_SUCCESS) {
                throw std::runtime_error("SyncManager: failed to create renderFinished semaphore");
            }
        }
    }

    void SyncManager::destroySwapchainSemaphores() {
        for (VkSemaphore s : render_finished_semaphores) {
            if (s != VK_NULL_HANDLE) {
                vkDestroySemaphore(device_manager->getDevice(), s, nullptr);
            }
        }
        render_finished_semaphores.clear();
    }
} // RtEngine
