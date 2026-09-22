#include "Composition.hpp"

#include "compute/CompositionRenderer.hpp"
#include "compute/CyclicalCellularAutomatonRenderer.hpp"
#include "compute/GlitchRenderer.hpp"
#include "compute/MandelbrotRenderer.hpp"
#include "compute/MandelbulbRenderer.hpp"
#include "CyclicalCellularAutomaton.hpp"
#include "EngineContext.hpp"
#include "Glitch.hpp"
#include "Mandelbrot.hpp"
#include "Mandelbulb.hpp"
#include "Scene.hpp"

#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>
#include <library/rave_visualizer/CompositionManager.hpp>
#include <logging/LogManager.hpp>

#include <string>
#include <vector>

namespace RtEngine
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<Composition>();
            return instance;
        }

        std::vector<std::string> visualizationOptions()
        {
            const auto& names = RaveVisualizer::visualizationTypeNames();
            return {names.begin(), names.end()};
        }

        RaveVisualizer::VisualizationType parseVisualization(const std::string& name)
        {
            const auto& names = RaveVisualizer::visualizationTypeNames();
            for (std::size_t i = 0; i < names.size(); ++i)
            {
                if (names[i] == name)
                {
                    return RaveVisualizer::visualizationTypeFromIndex(i);
                }
            }
            return RaveVisualizer::VisualizationType::CCA;
        }

        const std::string& visualizationName(RaveVisualizer::VisualizationType type)
        {
            return RaveVisualizer::visualizationTypeNames()[RaveVisualizer::visualizationTypeIndex(type)];
        }
    } // namespace

    Composition::Composition() = default;
    Composition::Composition(const std::shared_ptr<EngineContext>& context, const std::shared_ptr<Node>& node)
        : Component(context, node)
    {
    }
    Composition::~Composition() = default;

    void Composition::OnUpdate()
    {
        if (!composition_renderer && !tryInitialize())
        {
            return;
        }

        if (pending_resize_extent)
        {
            composition_renderer->handleResize(*pending_resize_extent);
            if (glitch_renderer)
            {
                glitch_renderer->handleResize(*pending_resize_extent);
            }
            pending_resize_extent.reset();
        }

        if (!manager)
        {
            tryBuildManager();
        }
        if (!manager)
        {
            return;
        }

        manager->pollInput();

        if (!update_gate.tick())
        {
            return;
        }

        // Only push the checkbox on transitions; the manager owns burst state
        // triggered by inputs and shouldn't be overwritten every tick.
        if (inversion_staccato != applied_inversion_staccato)
        {
            manager->setInversionStaccato(inversion_staccato);
            applied_inversion_staccato = inversion_staccato;
        }
        manager->setAnimate(animate);
        manager->setRotationIntervalSeconds(rotation_interval_s);

        // Dropdown changes are honored only when not auto-animating; they use
        // the same fade infrastructure as the automatic rotation.
        if (!animate && current_visualization != applied_visualization)
        {
            manager->TryChangeType(parseVisualization(current_visualization));
            applied_visualization = current_visualization;
        }

        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last_tick).count();
        last_tick = now;

        manager->tick(dt);

        // Keep the dropdown in sync with the automatic rotation.
        if (animate)
        {
            current_visualization = visualizationName(manager->state().current);
            applied_visualization = current_visualization;
        }

        const auto weights = manager->currentWeights();
        const auto set_active = [](const std::weak_ptr<Renderer>& weak, bool active)
        {
            if (const auto r = weak.lock())
            {
                r->setActive(active);
            }
        };
        set_active(cca_renderer, weights[0] > 0.0F);
        set_active(mandelbrot_renderer, weights[1] > 0.0F);
        set_active(mandelbulb_renderer, weights[2] > 0.0F);
    }

    void Composition::OnDestroy()
    {
        if (resize_callback_handle != 0 && context && context->swapchain_manager)
        {
            context->swapchain_manager->removeRecreateCallback(resize_callback_handle);
            resize_callback_handle = 0;
        }
    }

    bool Composition::tryInitialize()
    {
        if (!context || !context->scene_manager)
        {
            return false;
        }

        const auto mandelbrot_comp = context->scene_manager->getComponent<Mandelbrot>();
        const auto cca_comp = context->scene_manager->getComponent<CyclicalCellularAutomaton>();
        const auto mandelbulb_comp = context->scene_manager->getComponent<Mandelbulb>();
        if (!mandelbrot_comp || !cca_comp || !mandelbulb_comp)
        {
            logger()->warn(
                "Composition: scene must contain Mandelbrot, CyclicalCellularAutomaton and Mandelbulb components");
            return false;
        }

        const auto mandelbrot_out = mandelbrot_comp->getOutputConnector();
        const auto cca_out = cca_comp->getOutputConnector();
        const auto mandelbulb_out = mandelbulb_comp->getOutputConnector();
        if (!mandelbrot_out || !cca_out || !mandelbulb_out)
        {
            return false;
        }

        const auto rendering_manager = context->rendering_manager;
        const auto vulkan_context = rendering_manager->getVulkanContext();
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        composition_renderer =
            std::make_shared<CompositionRenderer>(vulkan_context, extent, cca_out, mandelbrot_out, mandelbulb_out);
        composition_renderer->init();
        rendering_manager->addComputeRenderer(composition_renderer, nullptr);

        resize_callback_handle = context->swapchain_manager->addRecreateCallback(
            [this](uint32_t width, uint32_t height) { pending_resize_extent = VkExtent2D{width, height}; });

        last_tick = std::chrono::steady_clock::now();
        return true;
    }

    void Composition::tryBuildManager()
    {
        const auto glitch_comp = context->scene_manager->getComponent<Glitch>();
        if (glitch_comp)
        {
            glitch_renderer = glitch_comp->getRenderer();
            if (!glitch_renderer)
            {
                return;
            }
        }

        const auto initial_palette = ::color::ColorPaletteFactory::create(::color::ColorPaletteName::Sunburn);
        manager = std::make_unique<RaveVisualizer::CompositionManager>(composition_renderer, glitch_comp,
            ::color::ColorPalette{initial_palette}, context ? context->input_manager : nullptr);

        const auto cca_comp = context->scene_manager->getComponent<CyclicalCellularAutomaton>();
        const auto mandelbrot_comp = context->scene_manager->getComponent<Mandelbrot>();
        const auto mandelbulb_comp = context->scene_manager->getComponent<Mandelbulb>();
        if (const auto r = cca_comp ? cca_comp->getRenderer() : nullptr)
        {
            cca_renderer = r;
            std::weak_ptr<CyclicalCellularAutomatonRenderer> weak = r;
            manager->addPaletteListener([weak](const ::color::ColorPalette& palette)
                {
                    if (const auto locked = weak.lock())
                    {
                        locked->setPalette(palette.colors);
                    }
                });
        }
        if (const auto r = mandelbrot_comp ? mandelbrot_comp->getRenderer() : nullptr)
        {
            mandelbrot_renderer = r;
            std::weak_ptr<MandelbrotRenderer> weak = r;
            manager->addPaletteListener([weak](const ::color::ColorPalette& palette)
                {
                    if (const auto locked = weak.lock())
                    {
                        locked->setPalette(palette.colors);
                    }
                });
        }
        if (const auto r = mandelbulb_comp ? mandelbulb_comp->getRenderer() : nullptr)
        {
            mandelbulb_renderer = r;
            std::weak_ptr<MandelbulbRenderer> weak = r;
            manager->addPaletteListener([weak](const ::color::ColorPalette& palette)
                {
                    if (const auto locked = weak.lock())
                    {
                        locked->setPalette(palette.colors);
                    }
                });
        }
    }

    std::shared_ptr<ImageConnector> Composition::getOutputConnector() const
    {
        return composition_renderer ? composition_renderer->getOutputConnector() : nullptr;
    }

    void Composition::initProperties(
        const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& /*update_flags*/)
    {
        if (current_visualization.empty())
        {
            current_visualization = visualizationName(RaveVisualizer::VisualizationType::CCA);
        }
        if (config->startChild(COMPONENT_NAME))
        {
            config->addSelection("visualization", &current_visualization, visualizationOptions());
            config->addBool("animate", &animate);
            config->addFloat("rotation_interval_s", &rotation_interval_s, 1.0F, 600.0F);
            config->addBool("inversion_staccato", &inversion_staccato);
            config->endChild();
        }
    }
} // namespace RtEngine
