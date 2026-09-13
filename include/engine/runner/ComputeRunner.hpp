#ifndef EDNA_ENGINE_COMPUTERUNNER_HPP
#define EDNA_ENGINE_COMPUTERUNNER_HPP
#include <memory>

#include "EngineContext.hpp"
#include "IRunner.hpp"
#include "PresentStage.hpp"
#include "RendererStack.hpp"
#include "SyncManager.hpp"

namespace RtEngine {
    class ComputeRunner : public IRunner {
    public:
        explicit ComputeRunner(std::shared_ptr<EngineContext> engine_context);

        bool isRunning() const override;
        void renderScene() override;
        void setUpdateFlags(const UpdateFlagsHandle &new_flags) const override;
        void initProperties(const std::shared_ptr<IProperties> &config, const UpdateFlagsHandle &update_flags) override;

    private:
        void drawFrame();
        void submitStack(uint32_t frame_idx);
        void handleResize() const;
        void waitForIdle() const;

        std::shared_ptr<EngineContext> engine_context;
        std::shared_ptr<VulkanContext> vulkan_context;
        std::shared_ptr<RendererStack> renderer_stack;
        std::shared_ptr<PresentStage> present_stage;
        std::shared_ptr<GuiRenderer> gui_renderer;
        std::shared_ptr<SyncManager> sync_manager;

        UpdateFlagsHandle update_flags;
        bool running = true;
    };
} // RtEngine

#endif //EDNA_ENGINE_COMPUTERUNNER_HPP
