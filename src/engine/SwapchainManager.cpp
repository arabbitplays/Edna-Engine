#include "../../include/engine/SwapchainManager.hpp"

#include <cassert>

namespace RtEngine {
    SwapchainManager::SwapchainManager(const std::shared_ptr<Swapchain> &swapchain) : swapchain(swapchain) {
        assert(swapchain != nullptr);
    }

    SwapchainManager::RecreateCallbackHandle SwapchainManager::addRecreateCallback(
            const std::function<void(uint32_t, uint32_t)> &func) {
        const RecreateCallbackHandle handle = next_callback_handle++;
        recreate_callbacks.emplace(handle, func);
        return handle;
    }

    void SwapchainManager::removeRecreateCallback(RecreateCallbackHandle handle) {
        recreate_callbacks.erase(handle);
    }

    void SwapchainManager::recreate() const {
        swapchain->recreate();
        VkExtent2D new_extent = swapchain->extent;
        for (const auto& [handle, callback] : recreate_callbacks) {
            callback(new_extent.width, new_extent.height);
        }
    }

    VkExtent2D SwapchainManager::getSwapchainExtent() const {
        return swapchain->extent;
    }
} // RtEngine

