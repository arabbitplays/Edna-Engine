#include "Mandelbrot.hpp"

#include "compute/MandelbrotRenderer.hpp"

#include <glm/glm.hpp>
#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>
#include <library/mandelbrot/animation/MandelbrotAnimationGenerator.hpp>
#include <library/mandelbrot/animation/MandelbrotAnimationRunner.hpp>
#include <library/mandelbrot/MandelbrotState.hpp>

using namespace color;

namespace RtEngine
{
    namespace
    {
        std::vector<glm::vec4> loadPaletteColors(const std::string& name)
        {
            const auto palette_name = ColorPaletteName::fromString(name, ColorPaletteName::Sunburn);
            return ColorPaletteFactory::create(palette_name).colors;
        }
    } // namespace

    void Mandelbrot::OnStart()
    {
        const std::shared_ptr<RenderingManager> rendering_manager = context->rendering_manager;
        const std::shared_ptr<VulkanContext> vulkan_context = rendering_manager->getVulkanContext();
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        const std::vector<glm::vec4> colors = loadPaletteColors(palette_name);
        applied_palette_name = palette_name;

        renderer = std::make_shared<MandelbrotRenderer>(vulkan_context, extent, colors, origin.x, origin.y, offset.x,
            offset.y, step_size, max_iterations, initial_number.x, initial_number.y, julia_mode);
        renderer->init();

        rendering_manager->addComputeRenderer(renderer, renderer->getOutputConnector());

        // Runner writes into the same component fields the UI reads, so
        // switching animate/manual just picks who owns the values.
        animation_runner = std::make_unique<::mandelbrot::MandelbrotAnimationRunner>([this](const glm::vec2& v)
            { offset = v; }, [this](float v) { step_size = v; }, [this](const glm::vec2& v) { initial_number = v; },
            ::mandelbrot::MandelbrotState{
                .offset = offset, .step_size = step_size, .initial = initial_number, .julia_mode = julia_mode});

        resize_callback_handle = context->swapchain_manager->addRecreateCallback(
            [this](uint32_t width, uint32_t height) { renderer->handleResize(VkExtent2D{width, height}); });
    }

    void Mandelbrot::OnDestroy()
    {
        if (resize_callback_handle != 0 && context && context->swapchain_manager)
        {
            context->swapchain_manager->removeRecreateCallback(resize_callback_handle);
            resize_callback_handle = 0;
        }
    }

    void Mandelbrot::OnUpdate()
    {
        if (!renderer)
        {
            return;
        }
        if (!update_gate.tick())
        {
            return;
        }

        if (animate && animation_runner)
        {
            const VkExtent2D extent = context->rendering_manager->getVulkanContext()->swapchain->extent;
            const float view_span = static_cast<float>(extent.width) * step_size;
            const glm::vec2 view_center = origin + offset * view_span;
            animation_runner->update(view_center, view_span, julia_mode);
        }
        else if (palette_name != applied_palette_name)
        {
            renderer->setPalette(loadPaletteColors(palette_name));
            applied_palette_name = palette_name;
        }

        renderer->setOrigin(origin.x, origin.y);
        renderer->setOffset(offset.x, offset.y);
        renderer->setStepSize(step_size);
        renderer->setMaxIterations(max_iterations);
        renderer->setInitial(initial_number.x, initial_number.y);
        renderer->setJuliaMode(julia_mode);
    }

    std::shared_ptr<ImageConnector> Mandelbrot::getOutputConnector() const
    {
        return renderer ? renderer->getOutputConnector() : nullptr;
    }

    void Mandelbrot::initProperties(
        const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& /*update_flags*/)
    {
        if (config->startChild(COMPONENT_NAME))
        {
            config->addVector("origin", &origin, -ORIGIN_BOUND, ORIGIN_BOUND);
            config->addVector("offset", &offset, -OFFSET_BOUND, OFFSET_BOUND);
            config->addFloat("step_size", &step_size);
            config->addUint("max_iterations", &max_iterations, MIN_MAX_ITERATIONS, MAX_MAX_ITERATIONS);
            config->addVector("initial_number", &initial_number, -INITIAL_BOUND, INITIAL_BOUND);
            config->addBool("julia_mode", &julia_mode);
            config->addBool("animate", &animate);
            config->addSelection("palette", &palette_name, ::color::ColorPaletteName::getAllNames());
            config->endChild();
        }
    }
} // namespace RtEngine
