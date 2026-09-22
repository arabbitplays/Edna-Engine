#include "Glitch.hpp"

#include <spdlog/spdlog.h>

#include "EngineContext.hpp"
#include "Node.hpp"
#include "Scene.hpp"
#include "compute/GlitchRenderer.hpp"

namespace RtEngine {
    Glitch::Glitch() = default;
    Glitch::Glitch(const std::shared_ptr<EngineContext>& context,
                   const std::shared_ptr<Node>& node)
        : Component(context, node) {}
    Glitch::~Glitch() = default;

    void Glitch::OnUpdate() {
        if (!renderer && !tryInitialize()) return;
        if (!update_gate.tick()) return;

        renderer->setShakePower(shake_power);
        renderer->setShakeRate(shake_rate);
        renderer->setShakeSpeed(shake_speed);
        renderer->setShakeBlockSize(shake_block_size);
        renderer->setShakeColorRate(shake_color_rate);
    }

    bool Glitch::tryInitialize() {
        if (!context || !context->scene_manager) return false;

        const auto scene = context->scene_manager->getCurrentScene();
        if (!scene) return false;

        const auto it = scene->nodes.find(source_node);
        if (it == scene->nodes.end()) {
            spdlog::warn("Glitch: source node '{}' not found in scene", source_node);
            return false;
        }

        std::shared_ptr<ImageConnector> input;
        for (const auto& comp : it->second->components) {
            if (auto out = comp->getOutputConnector()) {
                input = std::move(out);
                break;
            }
        }
        if (!input) return false;

        const auto rendering_manager = context->rendering_manager;
        const auto vulkan_context    = rendering_manager->getVulkanContext();
        const VkExtent2D extent      = vulkan_context->swapchain->extent;

        renderer = std::make_shared<GlitchRenderer>(vulkan_context, extent, input);
        renderer->init();
        rendering_manager->addComputeRenderer(renderer, renderer->getOutputConnector());

        return true;
    }

    std::shared_ptr<ImageConnector> Glitch::getOutputConnector() const {
        return renderer ? renderer->getOutputConnector() : nullptr;
    }

    void Glitch::initProperties(const std::shared_ptr<IProperties>& config,
                                const UpdateFlagsHandle&) {
        if (config->startChild(COMPONENT_NAME)) {
            config->addString("source_node", &source_node);
            config->addFloat("shake_power", &shake_power, 0.0f, 1.0f);
            config->addFloat("shake_rate", &shake_rate, 0.0f, 1.0f);
            config->addFloat("shake_speed", &shake_speed, 0.0f, 60.0f);
            config->addFloat("shake_block_size", &shake_block_size, 1.0f, 200.0f);
            config->addFloat("shake_color_rate", &shake_color_rate, 0.0f, 0.1f);
            config->endChild();
        }
    }
} // RtEngine
