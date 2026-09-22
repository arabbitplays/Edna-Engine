#ifndef EDNA_ENGINE_GLITCH_HPP
#define EDNA_ENGINE_GLITCH_HPP
#include "Component.hpp"

#include <FrameGate.hpp>
#include <memory>
#include <optional>
#include <string>

namespace RtEngine
{
    class GlitchRenderer;

    class Glitch : public Component
    {
    public:
        Glitch();
        Glitch(const std::shared_ptr<EngineContext>& context, const std::shared_ptr<Node>& node);
        ~Glitch() override;

        static inline const std::string COMPONENT_NAME = "Glitch";

        void OnStart() override
        {
        }
        void OnRender(DrawContext&) override
        {
        }
        void OnUpdate() override;
        void OnDestroy() override
        {
        }

        void initProperties(const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& update_flags) override;

        std::shared_ptr<GlitchRenderer> getRenderer() const
        {
            return renderer;
        }
        std::shared_ptr<ImageConnector> getOutputConnector() const override;

        // While an override is set, OnUpdate pushes the override values
        // instead of the ImGui/YAML-configured baseline. Clearing the
        // override restores the baseline on the next tick.
        void setShakeOverride(float power, float rate)
        {
            override_shake_power = power;
            override_shake_rate = rate;
        }
        void clearShakeOverride()
        {
            override_shake_power.reset();
            override_shake_rate.reset();
        }
        float baseShakePower() const
        {
            return shake_power;
        }
        float baseShakeRate() const
        {
            return shake_rate;
        }

    private:
        bool tryInitialize();

        std::string source_node = "Composition";

        float shake_power = 0.03f;
        float shake_rate = 0.2f;
        float shake_speed = 5.0f;
        float shake_block_size = 30.5f;
        float shake_color_rate = 0.01f;

        std::optional<float> override_shake_power;
        std::optional<float> override_shake_rate;

        std::shared_ptr<GlitchRenderer> renderer;

        FrameGate update_gate{30.0f};
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_GLITCH_HPP
