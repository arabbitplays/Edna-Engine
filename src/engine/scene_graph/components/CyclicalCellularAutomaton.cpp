#include "CyclicalCellularAutomaton.hpp"

#include "compute/CyclicalCellularAutomatonRenderer.hpp"

#include <glm/glm.hpp>
#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationGenerator.hpp>
#include <library/cellular_automaton/neighborhoods/NeighborhoodFactory.hpp>
#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>

using namespace cellular_automaton;

namespace RtEngine
{
    namespace
    {
        std::vector<glm::vec4> loadPaletteColors(const std::string& name)
        {
            const auto palette_name = ::color::ColorPaletteName::fromString(name, ::color::ColorPaletteName::Sunburn);
            return ::color::ColorPaletteFactory::create(palette_name).colors;
        }

        std::vector<glm::ivec2> loadNeighborhoodOffsets(const std::string& shape_name, uint32_t size)
        {
            const auto shape = NeighborhoodShape::fromString(shape_name, NeighborhoodShape::Box);
            return NeighborhoodFactory::create(shape, static_cast<int>(size)).offsets;
        }
    } // namespace

    void CyclicalCellularAutomaton::OnStart()
    {
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

        std::weak_ptr<CyclicalCellularAutomatonRenderer> weak_renderer = renderer;
        animation_runner = std::make_unique<CyclicalCellularAutomatonAnimationRunner>(
            [weak_renderer](float value)
            {
                if (const auto r = weak_renderer.lock())
                {
                    r->setMutationChance(value);
                }
            },
            [weak_renderer](const std::vector<glm::ivec2>& offsets)
            {
                if (const auto r = weak_renderer.lock())
                {
                    r->setNeighborhood(offsets);
                }
            },
            [weak_renderer](uint32_t value)
            {
                if (const auto r = weak_renderer.lock())
                {
                    r->setThreshold(value);
                }
            },
            mutation_chance);

        resize_callback_handle = context->swapchain_manager->addRecreateCallback(
            [this](uint32_t width, uint32_t height) { renderer->handleResize(VkExtent2D{width, height}); });
    }

    void CyclicalCellularAutomaton::OnDestroy()
    {
        if (resize_callback_handle != 0 && context && context->swapchain_manager)
        {
            context->swapchain_manager->removeRecreateCallback(resize_callback_handle);
            resize_callback_handle = 0;
        }
    }

    void CyclicalCellularAutomaton::OnUpdate()
    {
        if (!renderer)
        {
            return;
        }

        // Push setUpdate every frame so state advances exactly once per gate
        // tick — otherwise the flag latches on and the CA would step every
        // render frame between gate fires.
        const bool advance = update_gate.tick();
        renderer->setUpdate(advance);
        if (!advance)
        {
            return;
        }

        renderer->setUpdateChance(update_chance);

        if (animate && animation_runner)
        {
            animation_runner->update();
        }
        else
        {
            renderer->setThreshold(threshold);
            renderer->setMutationChance(mutation_chance);

            if (palette_name != applied_palette_name)
            {
                renderer->setPalette(loadPaletteColors(palette_name));
                applied_palette_name = palette_name;
            }

            if (neighborhood_shape != applied_neighborhood_shape || neighborhood_size != applied_neighborhood_size)
            {
                renderer->setNeighborhood(loadNeighborhoodOffsets(neighborhood_shape, neighborhood_size));
                applied_neighborhood_shape = neighborhood_shape;
                applied_neighborhood_size = neighborhood_size;
            }
        }
    }

    std::shared_ptr<ImageConnector> CyclicalCellularAutomaton::getOutputConnector() const
    {
        return renderer ? renderer->getOutputConnector() : nullptr;
    }

    void CyclicalCellularAutomaton::initProperties(
        const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& /*update_flags*/)
    {
        if (config->startChild(COMPONENT_NAME))
        {
            config->addUint("threshold", &threshold, 1U, 8U);
            config->addFloat("update_chance", &update_chance, 0.0F, 1.0F);
            config->addFloat("mutation_chance", &mutation_chance, 0.0F, 1.0F);
            config->addSelection("palette", &palette_name, ::color::ColorPaletteName::getAllNames());
            config->addSelection(
                "neighborhood_shape", &neighborhood_shape, cellular_automaton::NeighborhoodShape::getAllNames());
            config->addUint("neighborhood_size", &neighborhood_size, 1U, MAX_NEIGHBORHOOD_SIZE);
            config->addBool("animate", &animate);
            config->endChild();
        }
    }
} // namespace RtEngine
