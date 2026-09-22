#ifndef EDNA_ENGINE_RAVESTATE_HPP
#define EDNA_ENGINE_RAVESTATE_HPP
#include "VisualizationPhase.hpp"
#include "VisualizationType.hpp"

namespace RaveVisualizer
{
    struct RaveState
    {
        VisualizationPhase phase = VisualizationPhase::VISUALIZATION;

        // Which visualization is currently shown as the "front" input.
        VisualizationType current = VisualizationType::CCA;
        // Target visualization during a fade; equals `current` outside FADE.
        VisualizationType target = VisualizationType::CCA;

        float fade_progress = 0.0f;

        bool invert_color = false;
    };
} // namespace RaveVisualizer

#endif // EDNA_ENGINE_RAVESTATE_HPP
