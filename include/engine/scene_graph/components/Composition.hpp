#ifndef EDNA_ENGINE_COMPOSITION_HPP
#define EDNA_ENGINE_COMPOSITION_HPP
#include "Component.hpp"
#include "SwapchainManager.hpp"

#include <chrono>
#include <FrameGate.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vulkan/vulkan.h>

namespace RaveVisualizer
{
    class CompositionManager;
}

namespace RtEngine
{
    class CompositionRenderer;
    class GlitchRenderer;
    class Glitch;

    class Composition : public Component
    {
    public:
        Composition();
        Composition(const std::shared_ptr<EngineContext>& context, const std::shared_ptr<Node>& node);
        ~Composition() override;

        static inline const std::string COMPONENT_NAME = "Composition";

        void OnStart() override
        {
        }
        void OnRender(DrawContext&) override
        {
        }
        void OnUpdate() override;
        void OnDestroy() override;

        void initProperties(const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& update_flags) override;

        std::shared_ptr<ImageConnector> getOutputConnector() const override;

    private:
        bool tryInitialize();
        void tryBuildManager();

        bool inversion_staccato = false;

        std::shared_ptr<CompositionRenderer> composition_renderer;
        std::shared_ptr<GlitchRenderer> glitch_renderer;
        std::unique_ptr<RaveVisualizer::CompositionManager> manager;

        // Deferred to next OnUpdate: SwapchainManager fires resize callbacks
        // in unspecified order, so we can't assume our siblings have recreated
        // their outputs yet at callback time.
        std::optional<VkExtent2D> pending_resize_extent;

        std::chrono::steady_clock::time_point last_tick;
        FrameGate update_gate{30.0f};
        SwapchainManager::RecreateCallbackHandle resize_callback_handle = 0;
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_COMPOSITION_HPP
