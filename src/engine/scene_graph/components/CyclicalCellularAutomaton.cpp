#include "CyclicalCellularAutomaton.hpp"

#include <array>
#include <glm/glm.hpp>

#include <library/cellular_automaton/neighborhoods/NeighborhoodFactory.hpp>

#include "compute/CyclicalCellularAutomatonRenderer.hpp"

namespace RtEngine {
    namespace {
        std::vector<glm::ivec2> loadNeighborhoodOffsets(const std::string& shape_name, uint32_t size) {
            const auto shape = cellular_automaton::NeighborhoodShape::fromString(
                shape_name, cellular_automaton::NeighborhoodShape::Box);
            return cellular_automaton::NeighborhoodFactory::create(shape, static_cast<int>(size)).offsets;
        }
    }

    void CyclicalCellularAutomaton::OnStart() {
        const std::shared_ptr<RenderingManager> rendering_manager = context->rendering_manager;
        const std::shared_ptr<VulkanContext> vulkan_context = rendering_manager->getVulkanContext();
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        const std::vector<glm::vec4> colors = {
            glm::vec4{0.008f, 0.094f, 0.059f, 1.0f}, // #02180f
            glm::vec4{0.094f, 0.102f, 0.184f, 1.0f}, // #181a2f
            glm::vec4{0.027f, 0.176f, 0.212f, 1.0f}, // #072d36
            glm::vec4{0.114f, 0.243f, 0.173f, 1.0f}, // #1d3e2c
            glm::vec4{0.125f, 0.145f, 0.075f, 1.0f}, // #202513
            glm::vec4{0.271f, 0.153f, 0.161f, 1.0f}, // #452729
            glm::vec4{0.329f, 0.243f, 0.376f, 1.0f}, // #543e60
            glm::vec4{0.310f, 0.345f, 0.471f, 1.0f}, // #4f5878
            glm::vec4{0.439f, 0.173f, 0.122f, 1.0f}, // #702c1f
            glm::vec4{0.522f, 0.275f, 0.059f, 1.0f}, // #85460f
            glm::vec4{0.706f, 0.333f, 0.357f, 1.0f}, // #b4555b
            glm::vec4{0.902f, 0.576f, 0.459f, 1.0f}, // #e69375
            glm::vec4{0.604f, 0.380f, 0.278f, 1.0f}, // #9a6147
            glm::vec4{0.871f, 0.439f, 0.208f, 1.0f}, // #de7035
            glm::vec4{0.976f, 0.780f, 0.384f, 1.0f}, // #f9c762
            glm::vec4{1.000f, 1.000f, 0.620f, 1.0f}, // #ffff9e
        };

        const std::vector<glm::ivec2> offsets = loadNeighborhoodOffsets(neighborhood_shape, neighborhood_size);
        applied_neighborhood_shape = neighborhood_shape;
        applied_neighborhood_size = neighborhood_size;

        renderer = std::make_shared<CyclicalCellularAutomatonRenderer>(vulkan_context, extent, colors, offsets);
        renderer->init();
        renderer->setThreshold(threshold);
        renderer->setUpdateChance(update_chance);
        renderer->setMutationChance(mutation_chance);
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

        // Re-sync every frame so ImGui / YAML edits to the properties reach the shader
        // on the next dispatch without needing to touch the state texture.
        renderer->setThreshold(threshold);
        renderer->setUpdateChance(update_chance);
        renderer->setMutationChance(mutation_chance);

        if (neighborhood_shape != applied_neighborhood_shape || neighborhood_size != applied_neighborhood_size) {
            renderer->setNeighborhood(loadNeighborhoodOffsets(neighborhood_shape, neighborhood_size));
            applied_neighborhood_shape = neighborhood_shape;
            applied_neighborhood_size = neighborhood_size;
        }

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
            // Deliberately do not raise any update flag when these change — the setters are
            // re-applied every frame in OnUpdate, so mutating them just tweaks the next
            // dispatch, leaving the current state texture intact.
            config->addUint("threshold", &threshold, 1u, 8u);
            config->addFloat("update_chance", &update_chance, 0.0f, 1.0f);
            config->addFloat("mutation_chance", &mutation_chance, 0.0f, 1.0f);
            config->addSelection("neighborhood_shape", &neighborhood_shape,
                                 cellular_automaton::NeighborhoodShape::getAllNames());
            config->addUint("neighborhood_size", &neighborhood_size, 1u, MAX_NEIGHBORHOOD_SIZE);
            config->endChild();
        }
    }
} // RtEngine
