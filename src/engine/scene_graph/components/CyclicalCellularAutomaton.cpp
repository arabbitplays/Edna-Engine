#include "CyclicalCellularAutomaton.hpp"

#include <array>
#include <glm/glm.hpp>

#include "compute/CyclicalCellularAutomatonRenderer.hpp"

namespace RtEngine {
    void CyclicalCellularAutomaton::OnStart() {
        const std::shared_ptr<RenderingManager> rendering_manager = context->rendering_manager;
        const std::shared_ptr<VulkanContext> vulkan_context = rendering_manager->getVulkanContext();
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        const std::vector<glm::vec4> colors = {
            glm::vec4{0.00f, 0.00f, 0.00f, 1.0f},
            glm::vec4{0.14f, 0.07f, 0.00f, 1.0f},
            glm::vec4{0.29f, 0.14f, 0.00f, 1.0f},
            glm::vec4{0.43f, 0.21f, 0.00f, 1.0f},
            glm::vec4{0.57f, 0.29f, 0.00f, 1.0f},
            glm::vec4{0.71f, 0.36f, 0.00f, 1.0f},
            glm::vec4{0.86f, 0.43f, 0.00f, 1.0f},
            glm::vec4{1.00f, 0.50f, 0.00f, 1.0f},
            glm::vec4{1.00f, 0.56f, 0.08f, 1.0f},
            glm::vec4{1.00f, 0.63f, 0.15f, 1.0f},
            glm::vec4{1.00f, 0.69f, 0.23f, 1.0f},
            glm::vec4{1.00f, 0.75f, 0.30f, 1.0f},
            glm::vec4{1.00f, 0.81f, 0.38f, 1.0f},
            glm::vec4{1.00f, 0.88f, 0.45f, 1.0f},
            glm::vec4{1.00f, 0.94f, 0.53f, 1.0f},
            glm::vec4{1.00f, 1.00f, 0.60f, 1.0f},
        };

        renderer = std::make_shared<CyclicalCellularAutomatonRenderer>(vulkan_context, extent, colors);
        renderer->init();
        renderer->setThreshold(DEFAULT_THRESHOLD);
        renderer->setUpdate(false);

        rendering_manager->addComputeRenderer(renderer, renderer->getOutputConnector());

        resize_callback_handle = context->swapchain_manager->addRecreateCallback(
            [this](uint32_t width, uint32_t height) {
                renderer->handleResize(VkExtent2D{width, height});
            });

        last_update = std::chrono::steady_clock::now();
    }

    void CyclicalCellularAutomaton::OnDestroy() {
        if (resize_callback_handle != 0 && context && context->swapchain_manager) {
            context->swapchain_manager->removeRecreateCallback(resize_callback_handle);
            resize_callback_handle = 0;
        }
    }

    void CyclicalCellularAutomaton::OnUpdate() {
        if (!renderer) return;

        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - last_update).count();
        const bool should_update = elapsed >= UPDATE_INTERVAL_SECONDS;

        renderer->setUpdate(should_update);
        if (should_update) {
            last_update = now;
        }
    }

    void CyclicalCellularAutomaton::initProperties(const std::shared_ptr<IProperties>& config,
                                                    const UpdateFlagsHandle&) {
        if (config->startChild(COMPONENT_NAME)) {
            config->endChild();
        }
    }
} // RtEngine
