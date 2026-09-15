#include "CyclicalCellularAutomaton.hpp"

#include <glm/glm.hpp>

#include <library/cellular_automaton/colors/ColorPaletteFactory.hpp>
#include <library/cellular_automaton/neighborhoods/NeighborhoodFactory.hpp>

#include "compute/CyclicalCellularAutomatonRenderer.hpp"

using namespace cellular_automaton;

namespace RtEngine {
    namespace {
        std::vector<glm::vec4> loadPaletteColors(const std::string& name) {
            const auto palette_name = ColorPaletteName::fromString(name, ColorPaletteName::Sunburn);
            return ColorPaletteFactory::create(palette_name).colors;
        }

        std::vector<glm::ivec2> loadNeighborhoodOffsets(const std::string& shape_name, uint32_t size) {
            const auto shape = NeighborhoodShape::fromString(shape_name, NeighborhoodShape::Box);
            return NeighborhoodFactory::create(shape, static_cast<int>(size)).offsets;
        }
    }

    void CyclicalCellularAutomaton::OnStart() {
        const std::shared_ptr<RenderingManager> rendering_manager = context->rendering_manager;
        const std::shared_ptr<VulkanContext> vulkan_context = rendering_manager->getVulkanContext();
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        const std::vector<glm::vec4> colors = loadPaletteColors(palette_name);
        applied_palette_name = palette_name;

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

        if (palette_name != applied_palette_name) {
            renderer->setPalette(loadPaletteColors(palette_name));
            applied_palette_name = palette_name;
        }

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
            config->addSelection("palette", &palette_name, cellular_automaton::ColorPaletteName::getAllNames());
            config->addSelection("neighborhood_shape", &neighborhood_shape,
                                 cellular_automaton::NeighborhoodShape::getAllNames());
            config->addUint("neighborhood_size", &neighborhood_size, 1u, MAX_NEIGHBORHOOD_SIZE);
            config->endChild();
        }
    }
} // RtEngine
