#include <library/rave_visualizer/CompositionManager.hpp>

#include <algorithm>
#include <utility>

#include "Glitch.hpp"
#include "compute/CompositionRenderer.hpp"
#include "compute/GlitchRenderer.hpp"

namespace RaveVisualizer
{
    CompositionManager::CompositionManager(std::shared_ptr<RtEngine::CompositionRenderer> composition,
                                           std::shared_ptr<RtEngine::Glitch> glitch)
        : composition(std::move(composition)), glitch(std::move(glitch)) {
        rave_state.fade_progress = fadeValueFor(rave_state.current);
        pushToComposition();
    }

    void CompositionManager::tick(const float dt)
    {
        rotation_elapsed_s += dt;
        if (rotation_elapsed_s >= ROTATION_INTERVAL_S) {
            rotation_elapsed_s = 0.0F;
            const VisualizationType next = rave_state.current == VisualizationType::CCA
                                               ? VisualizationType::MANDELBROT
                                               : VisualizationType::CCA;
            TryChangeType(next);
        }

        if (rave_state.phase == VisualizationPhase::FADE) {
            fade_elapsed_s += dt;
            const float t = std::clamp(fade_elapsed_s / FADE_DURATION_S, 0.0F, 1.0F);
            const float from = fadeValueFor(rave_state.current);
            const float to   = fadeValueFor(rave_state.target);
            rave_state.fade_progress = from + ((to - from) * t);

            if (t >= 1.0F) {
                rave_state.current = rave_state.target;
                rave_state.fade_progress = fadeValueFor(rave_state.current);
                rave_state.phase = VisualizationPhase::VISUALIZATION;
                fade_elapsed_s = 0.0F;
            }
        }

        if (inversion_staccato_active) {
            staccato_elapsed_s += dt;
            const float period = 1.0F / INVERSION_STACCATO_HZ;
            while (staccato_elapsed_s >= period) {
                staccato_elapsed_s -= period;
                rave_state.invert_color = !rave_state.invert_color;
            }
        }

        pushToComposition();
    }

    void CompositionManager::TryChangeType(const VisualizationType new_type)
    {
        if (rave_state.phase == VisualizationPhase::FADE) { return;
}
        if (new_type == rave_state.current) { return;
}

        rave_state.target = new_type;
        fade_elapsed_s = 0.0F;
        rave_state.phase = VisualizationPhase::FADE;
    }

    void CompositionManager::setInversionStaccato(const bool active)
    {
        if (active == inversion_staccato_active) { return;
}
        inversion_staccato_active = active;
        staccato_elapsed_s = 0.0F;
        if (!active) {
            // Prevent the shader latching in an inverted state between bursts.
            rave_state.invert_color = false;
        }
    }

    namespace {
        template <typename Setter>
        void forwardToGlitch(const std::shared_ptr<RtEngine::Glitch>& glitch, Setter set) {
            if (!glitch) { return;
}
            const auto renderer = glitch->getRenderer();
            if (!renderer) { return;
}
            set(*renderer);
        }
    }

    void CompositionManager::setGlitchShakePower(const float v) {
        forwardToGlitch(glitch, [v](auto& r){ r.setShakePower(v); });
    }
    void CompositionManager::setGlitchShakeRate(const float v) {
        forwardToGlitch(glitch, [v](auto& r){ r.setShakeRate(v); });
    }
    void CompositionManager::setGlitchShakeSpeed(const float v) {
        forwardToGlitch(glitch, [v](auto& r){ r.setShakeSpeed(v); });
    }
    void CompositionManager::setGlitchShakeBlockSize(const float v) {
        forwardToGlitch(glitch, [v](auto& r){ r.setShakeBlockSize(v); });
    }
    void CompositionManager::setGlitchShakeColorRate(const float v) {
        forwardToGlitch(glitch, [v](auto& r){ r.setShakeColorRate(v); });
    }

    float CompositionManager::fadeValueFor(const VisualizationType type)
    {
        return type == VisualizationType::MANDELBROT ? 1.0F : 0.0F;
    }

    void CompositionManager::pushToComposition()
    {
        if (!composition) { return;
}
        composition->setFade(rave_state.fade_progress);
        composition->setInvertColor(rave_state.invert_color);
    }
} // RaveVisualizer
