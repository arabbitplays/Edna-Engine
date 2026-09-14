#include "../../../include/engine/runner/ComputeRunner.hpp"

#include <utility>

#include "UpdateFlagValue.hpp"

namespace RtEngine {
    ComputeRunner::ComputeRunner(std::shared_ptr<EngineContext> engine_context,
                                 std::shared_ptr<SceneManager> scene_manager)
        : engine_context(std::move(engine_context)),
          scene_manager(std::move(scene_manager)),
          vulkan_context(this->engine_context->rendering_manager->getVulkanContext()),
          renderer_stack(this->engine_context->rendering_manager->getRendererStack()),
          present_stage(this->engine_context->rendering_manager->getPresentStage()),
          gui_renderer(this->engine_context->rendering_manager->getGuiRenderer()),
          sync_manager(this->engine_context->sync_manager) {
        update_flags = std::make_shared<UpdateFlags>();
    }

    void ComputeRunner::loadScene(const std::string &scene_path) {
        scene_manager->loadScene(scene_path);
    }

    bool ComputeRunner::isRunning() const {
        return running;
    }

    void ComputeRunner::setUpdateFlags(const UpdateFlagsHandle &new_flags) const {
        update_flags->setFlags(new_flags);
    }

    void ComputeRunner::initProperties(const std::shared_ptr<IProperties> &config,
                                       const UpdateFlagsHandle &update_flags) {
        if (config->startChild("runner")) {
            if (config->addSelection("scene_name", &scene_name, scene_manager->getSceneNames())) {
                update_flags->setFlag(SCENE_UPDATE);
            }
            config->endChild();
        }
    }

    void ComputeRunner::renderScene() {
        if (update_flags->checkFlag(SCENE_UPDATE)) {
            loadScene(scene_manager->getScenePath(scene_name));
            update_flags->resetFlags();
        }
        if (const auto scene = scene_manager->getCurrentScene()) {
            scene->update();
        }
        drawFrame();
    }

    void ComputeRunner::drawFrame() {
        sync_manager->waitForNextFrameStart();

        const int32_t swapchain_image_idx = present_stage->acquireNextSwapchainImage();
        if (swapchain_image_idx < 0) {
            handleResize();
            return;
        }

        const uint32_t frame_idx = sync_manager->currentFrameInFlight();
        update_flags->resetFlags();

        submitStack(frame_idx);

        const uint32_t present_stage_idx = static_cast<uint32_t>(renderer_stack->getRenderers().size());
        const bool swapchain_out_of_date = present_stage->submitAndPresent(
            present_stage_idx, renderer_stack->getPresentConnector(), static_cast<uint32_t>(swapchain_image_idx));
        if (engine_context->rendering_manager->framebufferWasResized() || swapchain_out_of_date) {
            handleResize();
        }

        sync_manager->advanceFrame();
    }

    void ComputeRunner::submitStack(const uint32_t frame_idx) {
        const auto& renderers = renderer_stack->getRenderers();
        for (size_t i = 0; i < renderers.size(); i++) {
            VkCommandBuffer cmd = renderers[i]->recordCommandBuffer(frame_idx);
            sync_manager->submitStage(
                static_cast<uint32_t>(i),
                vulkan_context->device_manager->getQueue(renderers[i]->queueType()),
                cmd);
        }
    }

    void ComputeRunner::waitForIdle() const {
        vulkan_context->device_manager->waitForIdle();
    }

    void ComputeRunner::handleResize() const {
        waitForIdle();
        engine_context->swapchain_manager->recreate();
        gui_renderer->recreateFramebuffer();
    }
} // RtEngine
