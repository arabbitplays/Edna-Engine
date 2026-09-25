#ifndef EDNA_ENGINE_COMPOSITIONMANAGER_HPP
#define EDNA_ENGINE_COMPOSITIONMANAGER_HPP

#include "RaveState.hpp"
#include "VisualizationType.hpp"

#include <array>
#include <functional>
#include <library/color/ColorPalette.hpp>
#include <library/rave_visualizer/PaletteAnimationRunner.hpp>
#include <memory>
#include <vector>

namespace RtEngine
{
    class CompositionRenderer;
    class Glitch;
    class InputManager;
} // namespace RtEngine

namespace RaveVisualizer
{
    class CompositionManager
    {
    public:
        static constexpr float DEFAULT_ROTATION_INTERVAL_S = 60.0f;
        static constexpr float FADE_DURATION_S = 2.0f;
        static constexpr float INVERSION_STACCATO_HZ = 10.0f;

        // Duration the key-triggered inversion burst runs for.
        static constexpr float INVERSION_STACCATO_DURATION_S = 3.0f;

        // Ramp caps + growth rate for the key-triggered glitch intensification.
        static constexpr float GLITCH_RAMP_POWER_MAX = 0.4f;
        static constexpr float GLITCH_RAMP_RATE_MAX = 1.0f;
        static constexpr float GLITCH_RAMP_POWER_PER_S = 0.05f;
        static constexpr float GLITCH_RAMP_RATE_PER_S = 0.2f;

        using PaletteListener = PaletteAnimationRunner::Listener;

        CompositionManager(std::shared_ptr<RtEngine::CompositionRenderer> composition,
            std::shared_ptr<RtEngine::Glitch> glitch, ::color::ColorPalette initial_palette,
            std::shared_ptr<RtEngine::InputManager> input_manager = nullptr);

        // Registration is delegated to the palette runner; listeners fire
        // immediately with the current palette and then on every step.
        void addPaletteListener(PaletteListener listener);

        // Poll the input manager and latch any pending key events into the
        // manager's own buffer. Call every render frame so events aren't
        // dropped when tick() runs on a slower frame gate.
        void pollInput();

        void tick(float dt);

        // Triggers a timed staccato burst (INVERSION_STACCATO_DURATION_S).
        void triggerInversionStaccato();

        // Starts a monotonically-growing intensification of the glitch shake
        // power and rate. Idempotent: pressing repeatedly keeps the ramp
        // running from wherever it currently is.
        void startGlitchRamp();
        // Restores the glitch to its baseline configured values.
        void resetGlitchRamp();

        void TryChangeType(VisualizationType new_type);

        // When true, the manager cycles visualizations on its own timer; when
        // false, the type only changes on TryChangeType calls.
        void setAnimate(bool value)
        {
            animate = value;
        }
        bool isAnimate() const
        {
            return animate;
        }

        void setRotationIntervalSeconds(float seconds)
        {
            rotation_interval_s = seconds;
        }
        float rotationIntervalSeconds() const
        {
            return rotation_interval_s;
        }

        void setInversionStaccato(bool active);
        bool isInversionStaccatoActive() const
        {
            return inversion_staccato_active;
        }

        // No-op if the Glitch component is null or hasn't produced its renderer.
        void setGlitchShakePower(float v);
        void setGlitchShakeRate(float v);
        void setGlitchShakeSpeed(float v);
        void setGlitchShakeBlockSize(float v);
        void setGlitchShakeColorRate(float v);

        const RaveState& state() const
        {
            return rave_state;
        }

        std::array<float, VISUALIZATION_TYPE_COUNT> currentWeights() const;

    private:
        void pushToComposition();

        void handleInput();

        std::shared_ptr<RtEngine::CompositionRenderer> composition;
        std::shared_ptr<RtEngine::Glitch> glitch;
        std::shared_ptr<RtEngine::InputManager> input_manager;
        PaletteAnimationRunner palette_runner;
        RaveState rave_state;

        bool animate = true;
        float rotation_interval_s = DEFAULT_ROTATION_INTERVAL_S;
        float rotation_elapsed_s = 0.0f;
        float fade_elapsed_s = 0.0f;

        bool inversion_staccato_active = false;
        float staccato_elapsed_s = 0.0f;
        // Nonzero while the key-triggered burst is playing out.
        float staccato_remaining_s = 0.0f;

        bool glitch_ramp_active = false;
        float glitch_ramp_power = 0.0f;
        float glitch_ramp_rate = 0.0f;

        // Latched by pollInput() (every frame), drained by handleInput() on
        // the throttled tick, so key presses that happen between ticks aren't
        // lost to the input manager's per-frame reset.
        bool pending_trigger_staccato = false;
        bool pending_start_ramp = false;
        bool pending_reset_ramp = false;
    };
} // namespace RaveVisualizer

#endif // EDNA_ENGINE_COMPOSITIONMANAGER_HPP
