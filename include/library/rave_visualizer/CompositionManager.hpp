#ifndef EDNA_ENGINE_COMPOSITIONMANAGER_HPP
#define EDNA_ENGINE_COMPOSITIONMANAGER_HPP

#include "RaveState.hpp"
#include "VisualizationType.hpp"

#include <array>
#include <functional>
#include <library/color/ColorPalette.hpp>
#include <library/color/PaletteAnimationRunner.hpp>
#include <memory>

namespace RtEngine
{
    class CompositionRenderer;
    class Glitch;
} // namespace RtEngine

namespace RaveVisualizer
{
    class CompositionManager
    {
    public:
        static constexpr float DEFAULT_ROTATION_INTERVAL_S = 60.0f;
        static constexpr float FADE_DURATION_S = 2.0f;
        static constexpr float INVERSION_STACCATO_HZ = 10.0f;

        using PaletteListener = std::function<void(const ::color::ColorPalette&)>;

        CompositionManager(std::shared_ptr<RtEngine::CompositionRenderer> composition,
            std::shared_ptr<RtEngine::Glitch> glitch, ::color::ColorPalette initial_palette);

        // Listeners are invoked immediately with the current palette on
        // registration, and afterwards on every palette animation step.
        void addPaletteListener(PaletteListener listener);

        void tick(float dt);

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

        std::shared_ptr<RtEngine::CompositionRenderer> composition;
        std::shared_ptr<RtEngine::Glitch> glitch;
        ::color::PaletteAnimationRunner palette_runner;
        RaveState rave_state;

        bool animate = true;
        float rotation_interval_s = DEFAULT_ROTATION_INTERVAL_S;
        float rotation_elapsed_s = 0.0f;
        float fade_elapsed_s = 0.0f;

        bool inversion_staccato_active = false;
        float staccato_elapsed_s = 0.0f;
    };
} // namespace RaveVisualizer

#endif // EDNA_ENGINE_COMPOSITIONMANAGER_HPP
