#ifndef EDNA_ENGINE_GAUSSIANBLUR_HPP
#define EDNA_ENGINE_GAUSSIANBLUR_HPP
#include "Component.hpp"
#include "SwapchainManager.hpp"

#include <FrameGate.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace RtEngine
{
    class GaussianBlurRenderer;

    // Two-pass separable Gaussian blur wrapped as a scene component. Sources
    // its input image from `source_node`'s first component with an output
    // connector (mirrors the Glitch discovery pattern), registers a
    // horizontal and a vertical GaussianBlurRenderer into the compute stack,
    // and exposes the vertical pass output for downstream consumers.
    class GaussianBlur : public Component
    {
    public:
        GaussianBlur();
        GaussianBlur(const std::shared_ptr<EngineContext>& context, const std::shared_ptr<Node>& node);
        ~GaussianBlur() override;

        static inline const std::string COMPONENT_NAME = "GaussianBlur";

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

        // External gate (e.g. CompositionManager). When false, both passes
        // run as pass-throughs so the pipeline stays live and downstream
        // consumers still see a fresh copy of the source frame.
        void setActive(bool value)
        {
            active = value;
        }

    private:
        static constexpr uint32_t MIN_RADIUS = 0u;
        static constexpr uint32_t MAX_RADIUS = 64u;
        static constexpr uint32_t DEFAULT_RADIUS = 12u;

        bool tryInitialize();

        std::string source_node = "CyclicalCA";
        uint32_t radius = DEFAULT_RADIUS;
        bool active = true;

        std::shared_ptr<GaussianBlurRenderer> horizontal_renderer;
        std::shared_ptr<GaussianBlurRenderer> vertical_renderer;

        FrameGate update_gate{30.0f};
        SwapchainManager::RecreateCallbackHandle resize_callback_handle = 0;
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_GAUSSIANBLUR_HPP
