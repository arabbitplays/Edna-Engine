#ifndef VULKAN_RAYTRACING_SWAPCHAINMANAGER_HPP
#define VULKAN_RAYTRACING_SWAPCHAINMANAGER_HPP
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <Swapchain.hpp>

namespace RtEngine {
    class SwapchainManager {
    public:
        using RecreateCallbackHandle = uint64_t;

        SwapchainManager() = default;
        explicit SwapchainManager(const std::shared_ptr<Swapchain> &swapchain);

        RecreateCallbackHandle addRecreateCallback(const std::function<void(uint32_t, uint32_t)> &func);
        void removeRecreateCallback(RecreateCallbackHandle handle);

        void recreate() const;

        VkExtent2D getSwapchainExtent() const;

    private:
        std::shared_ptr<Swapchain> swapchain;
        std::unordered_map<RecreateCallbackHandle, std::function<void(uint32_t, uint32_t)>> recreate_callbacks;
        RecreateCallbackHandle next_callback_handle = 1;
    };
} // RtEngine

#endif //VULKAN_RAYTRACING_SWAPCHAINMANAGER_HPP