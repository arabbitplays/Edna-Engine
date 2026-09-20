#include <library/rave_visualizer/CompositionManager.hpp>

#include <algorithm>
#include <utility>

#include "compute/CompositionRenderer.hpp"

namespace RaveVisualizer
{
    CompositionManager::CompositionManager(std::shared_ptr<RtEngine::CompositionRenderer> renderer)
        : renderer(std::move(renderer)) {
        rave_state.fade_progress = fadeValueFor(rave_state.current);
        pushToRenderer();
    }

    void CompositionManager::tick(const float dt)
    {
        rotation_elapsed_s += dt;
        if (rotation_elapsed_s >= ROTATION_INTERVAL_S) {
            rotation_elapsed_s = 0.0f;
            const VisualizationType next = rave_state.current == VisualizationType::CCA
                                               ? VisualizationType::MANDELBROT
                                               : VisualizationType::CCA;
            TryChangeType(next);
        }

        if (rave_state.phase == VisualizationPhase::FADE) {
            fade_elapsed_s += dt;
            const float t = std::clamp(fade_elapsed_s / FADE_DURATION_S, 0.0f, 1.0f);
            const float from = fadeValueFor(rave_state.current);
            const float to   = fadeValueFor(rave_state.target);
            rave_state.fade_progress = from + (to - from) * t;

            if (t >= 1.0f) {
                rave_state.current = rave_state.target;
                rave_state.fade_progress = fadeValueFor(rave_state.current);
                rave_state.phase = VisualizationPhase::VISUALIZATION;
                fade_elapsed_s = 0.0f;
            }
        }

        if (inversion_staccato_active) {
            staccato_elapsed_s += dt;
            const float period = 1.0f / INVERSION_STACCATO_HZ;
            while (staccato_elapsed_s >= period) {
                staccato_elapsed_s -= period;
                rave_state.invert_color = !rave_state.invert_color;
            }
        }

        pushToRenderer();
    }

    void CompositionManager::TryChangeType(const VisualizationType new_type)
    {
        if (rave_state.phase == VisualizationPhase::FADE) return;
        if (new_type == rave_state.current) return;

        rave_state.target = new_type;
        fade_elapsed_s = 0.0f;
        rave_state.phase = VisualizationPhase::FADE;
    }

    void CompositionManager::setInversionStaccato(const bool active)
    {
        if (active == inversion_staccato_active) return;
        inversion_staccato_active = active;
        staccato_elapsed_s = 0.0f;
        if (!active) {
            // Restore un-inverted output when disarming so the shader doesn't
            // latch in an inverted state between bursts.
            rave_state.invert_color = false;
        }
    }

    float CompositionManager::fadeValueFor(const VisualizationType type)
    {
        // input_a = CCA -> fade 0.0, input_b = MANDELBROT -> fade 1.0.
        return type == VisualizationType::MANDELBROT ? 1.0f : 0.0f;
    }

    void CompositionManager::pushToRenderer()
    {
        if (!renderer) return;
        renderer->setFade(rave_state.fade_progress);
        renderer->setInvertColor(rave_state.invert_color);
    }
} // RaveVisualizer
