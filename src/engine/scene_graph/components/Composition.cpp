#include "Composition.hpp"

#include <spdlog/spdlog.h>

#include <library/rave_visualizer/CompositionManager.hpp>

#include "CyclicalCellularAutomaton.hpp"
#include "EngineContext.hpp"
#include "Glitch.hpp"
#include "Mandelbrot.hpp"
#include "Scene.hpp"
#include "compute/CompositionRenderer.hpp"
#include "compute/GlitchRenderer.hpp"

namespace RtEngine {
    Composition::Composition() = default;
    Composition::Composition(const std::shared_ptr<EngineContext>& context,
                             const std::shared_ptr<Node>& node)
        : Component(context, node) {}
    Composition::~Composition() = default;

    void Composition::OnUpdate() {
        if (!composition_renderer && !tryInitialize()) return;

        if (pending_resize_extent) {
            composition_renderer->handleResize(*pending_resize_extent);
            if (glitch_renderer) glitch_renderer->handleResize(*pending_resize_extent);
            pending_resize_extent.reset();
        }

        if (!manager) tryBuildManager();
        if (!manager) return;

        manager->setInversionStaccato(inversion_staccato);

        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last_tick).count();
        last_tick = now;

        manager->tick(dt);
    }

    void Composition::OnDestroy() {
        if (resize_callback_handle != 0 && context && context->swapchain_manager) {
            context->swapchain_manager->removeRecreateCallback(resize_callback_handle);
            resize_callback_handle = 0;
        }
    }

    bool Composition::tryInitialize() {
        if (!context || !context->scene_manager) return false;

        const auto mandelbrot_comp = context->scene_manager->getComponent<Mandelbrot>();
        const auto cca_comp        = context->scene_manager->getComponent<CyclicalCellularAutomaton>();
        if (!mandelbrot_comp || !cca_comp) {
            spdlog::warn("Composition: scene must contain a Mandelbrot and a CyclicalCellularAutomaton component");
            return false;
        }

        const auto mandelbrot_out = mandelbrot_comp->getOutputConnector();
        const auto cca_out        = cca_comp->getOutputConnector();
        if (!mandelbrot_out || !cca_out) return false;

        const auto rendering_manager = context->rendering_manager;
        const auto vulkan_context    = rendering_manager->getVulkanContext();
        const VkExtent2D extent      = vulkan_context->swapchain->extent;

        composition_renderer = std::make_shared<CompositionRenderer>(
            vulkan_context, extent, cca_out, mandelbrot_out);
        composition_renderer->init();
        rendering_manager->addComputeRenderer(composition_renderer, nullptr);

        resize_callback_handle = context->swapchain_manager->addRecreateCallback(
            [this](uint32_t width, uint32_t height) {
                pending_resize_extent = VkExtent2D{width, height};
            });

        last_tick = std::chrono::steady_clock::now();
        return true;
    }

    void Composition::tryBuildManager() {
        const auto glitch_comp = context->scene_manager->getComponent<Glitch>();
        if (glitch_comp) {
            glitch_renderer = glitch_comp->getRenderer();
            if (!glitch_renderer) return;
        }
        manager = std::make_unique<RaveVisualizer::CompositionManager>(
            composition_renderer, glitch_comp);
    }

    std::shared_ptr<ImageConnector> Composition::getOutputConnector() const {
        return composition_renderer ? composition_renderer->getOutputConnector() : nullptr;
    }

    void Composition::initProperties(const std::shared_ptr<IProperties>& config,
                                     const UpdateFlagsHandle&) {
        if (config->startChild(COMPONENT_NAME)) {
            config->addBool("inversion_staccato", &inversion_staccato);
            config->endChild();
        }
    }
} // RtEngine
