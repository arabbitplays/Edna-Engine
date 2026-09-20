#include <library/rave_visualizer/CompositionManager.hpp>

namespace RaveVisualizer
{
    void CompositionManager::TryChangeType(const VisualizationType new_type)
    {
        if (rave_state.phase == VisualizationPhase::FADE) return;
        if (new_type == rave_state.current) return;

        rave_state.target = new_type;
        rave_state.fade_progress = 0.0f;
        rave_state.phase = VisualizationPhase::FADE;
    }
} // RaveVisualizer
