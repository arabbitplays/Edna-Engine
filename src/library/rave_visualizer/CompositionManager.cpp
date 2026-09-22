#include "compute/CompositionRenderer.hpp"
#include "compute/GlitchRenderer.hpp"
#include "Glitch.hpp"
#include "InputManager.hpp"

#include <algorithm>
#include <library/rave_visualizer/CompositionManager.hpp>
#include <utility>

namespace RaveVisualizer
{
    CompositionManager::CompositionManager(std::shared_ptr<RtEngine::CompositionRenderer> composition,
        std::shared_ptr<RtEngine::Glitch> glitch, ::color::ColorPalette initial_palette,
        std::shared_ptr<RtEngine::InputManager> input_manager)
        : composition(std::move(composition)), glitch(std::move(glitch)), input_manager(std::move(input_manager)),
          palette_runner(std::move(initial_palette))
    {
        pushToComposition();
    }

    void CompositionManager::addPaletteListener(PaletteListener listener)
    {
        palette_runner.addListener(std::move(listener));
    }

    void CompositionManager::tick(const float dt)
    {
        handleInput();

        if (staccato_remaining_s > 0.0F)
        {
            staccato_remaining_s -= dt;
            if (staccato_remaining_s <= 0.0F)
            {
                staccato_remaining_s = 0.0F;
                setInversionStaccato(false);
                resetGlitchRamp();
            }
        }

        if (animate)
        {
            rotation_elapsed_s += dt;
            if (rotation_elapsed_s >= rotation_interval_s)
            {
                rotation_elapsed_s = 0.0F;
                const std::size_t current_index = visualizationTypeIndex(rave_state.current);
                const std::size_t next_index = (current_index + 1) % VISUALIZATION_TYPE_COUNT;
                TryChangeType(visualizationTypeFromIndex(next_index));
            }
        }
        else
        {
            rotation_elapsed_s = 0.0F;
        }

        if (rave_state.phase == VisualizationPhase::FADE)
        {
            fade_elapsed_s += dt;
            const float t = std::clamp(fade_elapsed_s / FADE_DURATION_S, 0.0F, 1.0F);
            rave_state.fade_progress = t;

            if (t >= 1.0F)
            {
                rave_state.current = rave_state.target;
                rave_state.fade_progress = 0.0F;
                rave_state.phase = VisualizationPhase::VISUALIZATION;
                fade_elapsed_s = 0.0F;
            }
        }

        if (inversion_staccato_active)
        {
            staccato_elapsed_s += dt;
            const float period = 1.0F / INVERSION_STACCATO_HZ;
            while (staccato_elapsed_s >= period)
            {
                staccato_elapsed_s -= period;
                rave_state.invert_color = !rave_state.invert_color;
            }
        }

        if (glitch_ramp_active && glitch)
        {
            glitch_ramp_power = std::min(GLITCH_RAMP_POWER_MAX,
                std::max(glitch_ramp_power, glitch->baseShakePower()) + GLITCH_RAMP_POWER_PER_S * dt);
            glitch_ramp_rate = std::min(GLITCH_RAMP_RATE_MAX,
                std::max(glitch_ramp_rate, glitch->baseShakeRate()) + GLITCH_RAMP_RATE_PER_S * dt);
            glitch->setShakeOverride(glitch_ramp_power, glitch_ramp_rate);
        }

        palette_runner.update();

        pushToComposition();
    }

    void CompositionManager::TryChangeType(const VisualizationType new_type)
    {
        if (rave_state.phase == VisualizationPhase::FADE)
        {
            return;
        }
        if (new_type == rave_state.current)
        {
            return;
        }

        rave_state.target = new_type;
        rave_state.fade_progress = 0.0F;
        fade_elapsed_s = 0.0F;
        rave_state.phase = VisualizationPhase::FADE;
    }

    void CompositionManager::triggerInversionStaccato()
    {
        staccato_remaining_s = INVERSION_STACCATO_DURATION_S;
        setInversionStaccato(true);
    }

    void CompositionManager::startGlitchRamp()
    {
        if (!glitch_ramp_active)
        {
            glitch_ramp_active = true;
            glitch_ramp_power = 0.0F;
            glitch_ramp_rate = 0.0F;
        }
    }

    void CompositionManager::resetGlitchRamp()
    {
        glitch_ramp_active = false;
        glitch_ramp_power = 0.0F;
        glitch_ramp_rate = 0.0F;
        if (glitch)
        {
            glitch->clearShakeOverride();
        }
    }

    void CompositionManager::pollInput()
    {
        if (!input_manager)
        {
            return;
        }

        if (input_manager->getKeyDown(RtEngine::Keycode::NUM_3))
        {
            pending_trigger_staccato = true;
        }
        if (input_manager->getKeyDown(RtEngine::Keycode::NUM_2))
        {
            pending_start_ramp = true;
        }
        if (input_manager->getKeyDown(RtEngine::Keycode::NUM_1))
        {
            pending_reset_ramp = true;
        }
    }

    void CompositionManager::handleInput()
    {
        if (pending_trigger_staccato)
        {
            triggerInversionStaccato();
            pending_trigger_staccato = false;
        }
        if (pending_start_ramp)
        {
            startGlitchRamp();
            pending_start_ramp = false;
        }
        if (pending_reset_ramp)
        {
            resetGlitchRamp();
            pending_reset_ramp = false;
        }
    }

    void CompositionManager::setInversionStaccato(const bool active)
    {
        if (active == inversion_staccato_active)
        {
            return;
        }
        inversion_staccato_active = active;
        staccato_elapsed_s = 0.0F;
        if (!active)
        {
            // Prevent the shader latching in an inverted state between bursts.
            rave_state.invert_color = false;
        }
    }

    namespace
    {
        template <typename Setter> void forwardToGlitch(const std::shared_ptr<RtEngine::Glitch>& glitch, Setter set)
        {
            if (!glitch)
            {
                return;
            }
            const auto renderer = glitch->getRenderer();
            if (!renderer)
            {
                return;
            }
            set(*renderer);
        }
    } // namespace

    void CompositionManager::setGlitchShakePower(const float v)
    {
        forwardToGlitch(glitch, [v](auto& r) { r.setShakePower(v); });
    }
    void CompositionManager::setGlitchShakeRate(const float v)
    {
        forwardToGlitch(glitch, [v](auto& r) { r.setShakeRate(v); });
    }
    void CompositionManager::setGlitchShakeSpeed(const float v)
    {
        forwardToGlitch(glitch, [v](auto& r) { r.setShakeSpeed(v); });
    }
    void CompositionManager::setGlitchShakeBlockSize(const float v)
    {
        forwardToGlitch(glitch, [v](auto& r) { r.setShakeBlockSize(v); });
    }
    void CompositionManager::setGlitchShakeColorRate(const float v)
    {
        forwardToGlitch(glitch, [v](auto& r) { r.setShakeColorRate(v); });
    }

    std::array<float, VISUALIZATION_TYPE_COUNT> CompositionManager::currentWeights() const
    {
        std::array<float, VISUALIZATION_TYPE_COUNT> weights{};
        weights.fill(0.0F);
        const std::size_t current_index = visualizationTypeIndex(rave_state.current);
        if (rave_state.phase == VisualizationPhase::FADE)
        {
            const std::size_t target_index = visualizationTypeIndex(rave_state.target);
            weights[current_index] = 1.0F - rave_state.fade_progress;
            weights[target_index] = rave_state.fade_progress;
        }
        else
        {
            weights[current_index] = 1.0F;
        }
        return weights;
    }

    void CompositionManager::pushToComposition()
    {
        if (!composition)
        {
            return;
        }
        const auto weights = currentWeights();
        composition->setWeights(weights[0], weights[1], weights[2]);
        composition->setInvertColor(rave_state.invert_color);
    }
} // namespace RaveVisualizer
