#ifndef EDNA_ENGINE_COMPOSITIONMANAGER_HPP
#define EDNA_ENGINE_COMPOSITIONMANAGER_HPP

#include <memory>

#include "RaveState.hpp"
#include "VisualizationType.hpp"

namespace RtEngine { class CompositionRenderer; }

namespace RaveVisualizer
{
    class CompositionManager
    {
    public:
        // Seconds between automatic visualization changes.
        static constexpr float ROTATION_INTERVAL_S = 60.0f;
        static constexpr float FADE_DURATION_S = 2.0f;
        static constexpr float INVERSION_STACCATO_HZ = 10.0f;

        explicit CompositionManager(std::shared_ptr<RtEngine::CompositionRenderer> renderer);

        void tick(float dt);

        void TryChangeType(VisualizationType new_type);

        void setInversionStaccato(bool active);
        bool isInversionStaccatoActive() const { return inversion_staccato_active; }

        const RaveState& state() const { return rave_state; }

    private:
        // Fixed binding: input_a = CCA, input_b = MANDELBROT. Returns the
        // fade value in [0,1] that displays `type` fully.
        static float fadeValueFor(VisualizationType type);

        void pushToRenderer();

        std::shared_ptr<RtEngine::CompositionRenderer> renderer;
        RaveState rave_state;

        float rotation_elapsed_s = 0.0f;
        float fade_elapsed_s     = 0.0f;

        bool  inversion_staccato_active = false;
        float staccato_elapsed_s        = 0.0f;
    };
} // RaveVisualizer

#endif //EDNA_ENGINE_COMPOSITIONMANAGER_HPP
