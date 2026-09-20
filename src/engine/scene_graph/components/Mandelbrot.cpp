#include "Mandelbrot.hpp"

#include <cmath>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include <library/cellular_automaton/colors/ColorPaletteFactory.hpp>

#include "compute/MandelbrotRenderer.hpp"

using namespace cellular_automaton;

namespace RtEngine {
    namespace {
        std::vector<glm::vec4> loadPaletteColors(const std::string& name) {
            const auto palette_name = ColorPaletteName::fromString(name, ColorPaletteName::Sunburn);
            return ColorPaletteFactory::create(palette_name).colors;
        }
    }

    void Mandelbrot::OnStart() {
        const std::shared_ptr<RenderingManager> rendering_manager = context->rendering_manager;
        const std::shared_ptr<VulkanContext> vulkan_context = rendering_manager->getVulkanContext();
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        const std::vector<glm::vec4> colors = loadPaletteColors(palette_name);
        applied_palette_name = palette_name;

        renderer = std::make_shared<MandelbrotRenderer>(
            vulkan_context, extent, colors,
            static_cast<double>(origin.x),
            static_cast<double>(origin.y),
            static_cast<double>(offset.x),
            static_cast<double>(offset.y),
            static_cast<double>(step_size),
            max_iterations,
            static_cast<double>(initial_number.x),
            static_cast<double>(initial_number.y),
            julia_mode);
        renderer->init();

        rendering_manager->addComputeRenderer(renderer, renderer->getOutputConnector());

        resize_callback_handle = context->swapchain_manager->addRecreateCallback(
            [this](uint32_t width, uint32_t height) {
                renderer->handleResize(VkExtent2D{width, height});
            });
    }

    void Mandelbrot::OnDestroy() {
        if (resize_callback_handle != 0 && context && context->swapchain_manager) {
            context->swapchain_manager->removeRecreateCallback(resize_callback_handle);
            resize_callback_handle = 0;
        }
    }

    void Mandelbrot::OnUpdate() {
        if (!renderer) return;

        renderer->setOrigin(static_cast<double>(origin.x), static_cast<double>(origin.y));
        renderer->setOffset(static_cast<double>(offset.x), static_cast<double>(offset.y));
        renderer->setStepSize(static_cast<double>(step_size));
        renderer->setMaxIterations(max_iterations);
        renderer->setInitial(static_cast<double>(initial_number.x), static_cast<double>(initial_number.y));
        renderer->setJuliaMode(julia_mode);

        if (palette_name != applied_palette_name) {
            renderer->setPalette(loadPaletteColors(palette_name));
            applied_palette_name = palette_name;
        }

        last_entropy = renderer->readEntropy();
        SPDLOG_INFO("Mandelbrot entropy: {:.3f} bits (max {:.3f})",
            last_entropy, std::log2(static_cast<float>(MandelbrotRenderer::HISTOGRAM_BIN_COUNT)));
    }

    void Mandelbrot::initProperties(const std::shared_ptr<IProperties>& config,
                                     const UpdateFlagsHandle&) {
        if (config->startChild(COMPONENT_NAME)) {
            config->addVector("origin", &origin, -ORIGIN_BOUND, ORIGIN_BOUND);
            config->addVector("offset", &offset, -OFFSET_BOUND, OFFSET_BOUND);
            config->addFloat("step_size", &step_size);
            config->addUint("max_iterations", &max_iterations, MIN_MAX_ITERATIONS, MAX_MAX_ITERATIONS);
            config->addVector("initial_number", &initial_number, -INITIAL_BOUND, INITIAL_BOUND);
            config->addBool("julia_mode", &julia_mode);
            config->addSelection("palette", &palette_name, cellular_automaton::ColorPaletteName::getAllNames());
            config->endChild();
        }
    }
} // RtEngine
