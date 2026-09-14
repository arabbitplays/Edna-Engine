#include "../../../include/engine/runner/Runner.hpp"

#include "PathUtil.hpp"
#include "SceneWriter.hpp"
#include "UpdateFlagValue.hpp"

namespace RtEngine {
    Runner::Runner(std::shared_ptr<EngineContext> engine_context, const std::shared_ptr<SceneManager> &scene_manager)
        : engine_context(engine_context), scene_manager(scene_manager),
          raytracing_renderer(engine_context->rendering_manager->getRaytracingRenderer()),
          gui_renderer(engine_context->rendering_manager->getGuiRenderer()),
          renderer_stack(engine_context->rendering_manager->getRendererStack()),
          present_stage(engine_context->rendering_manager->getPresentStage()),
          sync_manager(engine_context->sync_manager) {
        update_flags = std::make_shared<UpdateFlags>();
    }

    std::string Runner::getScenePath() const {
        return scene_manager->getCurrentScene() != nullptr ? scene_manager->getCurrentScene()->path : "";
    }

    void Runner::loadScene(const std::string &scene_path) {
        std::shared_ptr<Scene> new_scene =
                scene_manager->loadScene(scene_path, raytracing_renderer->getMaterials());

        raytracing_renderer->loadScene(new_scene);

        SceneWriter writer;
        writer.writeScene(PathUtil::getFileName(scene_path), new_scene);
    }

    void Runner::renderScene() {
        if (update_flags->checkFlag(SCENE_UPDATE)) {
            loadScene(scene_manager->getScenePath(scene_name));
        }

        scene_manager->getCurrentScene()->update();
        std::shared_ptr<DrawContext> draw_context = createMainDrawContext();
        if (draw_context->targets.size() < 1)
            return;
        drawFrame(draw_context);
    }

    void Runner::setUpdateFlags(const UpdateFlagsHandle &new_flags) const {
        update_flags->setFlags(new_flags);
    }

    bool Runner::isRunning() const {
        return running;
    }

    void Runner::initProperties(const std::shared_ptr<IProperties> &config, const UpdateFlagsHandle& update_flags) {
        if (config->startChild("runner")) {
            if (config->addSelection("scene_name", &scene_name, scene_manager->getSceneNames())) {
                update_flags->setFlag(SCENE_UPDATE);
            }
            config->endChild();
        }
    }

    void Runner::drawFrame(const std::shared_ptr<DrawContext>& draw_context) {
        sync_manager->waitForNextFrameStart();

        const int32_t swapchain_image_idx = present_stage->acquireNextSwapchainImage();
        if (swapchain_image_idx < 0) {
            handleResize();
            return;
        }

        const uint32_t frame_idx = sync_manager->currentFrameInFlight();
        std::shared_ptr<RenderTarget> target = draw_context->targets[0]; // TODO handle multiple

        prepareFrame(draw_context, frame_idx);

        raytracing_renderer->writeRenderTarget(target);

        renderFrame(frame_idx, static_cast<uint32_t>(swapchain_image_idx), true);
        finishFrame(draw_context);
    }

    void Runner::prepareFrame(const std::shared_ptr<DrawContext> &draw_context, uint32_t frame_idx) {
        raytracing_renderer->writeResources(draw_context, update_flags, frame_idx);

        if (update_flags->checkFlag(TARGET_RESET)) {
            for (const auto& target : draw_context->targets) {
                target->resetAccumulatedFrames();
            }
        }
        update_flags->resetFlags();
    }

    void Runner::renderFrame(uint32_t frame_idx,
                             uint32_t swapchain_image_idx,
                             bool present) const {
        auto vulkan_context = engine_context->rendering_manager->getVulkanContext();
        const auto& renderers = renderer_stack->getRenderers();
        for (size_t i = 0; i < renderers.size(); i++) {
            VkCommandBuffer cmd = renderers[i]->recordCommandBuffer(frame_idx);
            sync_manager->submitStage(
                static_cast<uint32_t>(i),
                vulkan_context->device_manager->getQueue(renderers[i]->queueType()),
                cmd);
        }

        const uint32_t present_stage_idx = static_cast<uint32_t>(renderers.size());
        if (present) {
            const bool swapchain_out_of_date = renderer_stack->getPresentStage()->submitAndPresent(
                present_stage_idx, renderer_stack->getPresentConnector(), swapchain_image_idx);
            if (engine_context->rendering_manager->framebufferWasResized() || swapchain_out_of_date) {
                handleResize();
            }
        } else {
            sync_manager->skipStage(present_stage_idx);
        }
    }

    void Runner::finishFrame(const std::shared_ptr<DrawContext> &draw_context) const {
        // TODO merge this
        sync_manager->advanceFrame();
        draw_context->nextFrame();
    }

    void Runner::waitForIdle() const {
        engine_context->rendering_manager->getVulkanContext()->device_manager->waitForIdle();
    }

    void Runner::handleResize() const {
        waitForIdle();
        engine_context->swapchain_manager->recreate();
        gui_renderer->recreateFramebuffer();
    }

    std::shared_ptr<DrawContext> Runner::createMainDrawContext() const {
        auto draw_context = std::make_shared<DrawContext>();
        scene_manager->getCurrentScene()->fillDrawContext(draw_context);
        return draw_context;
    }
} // RtEngine
