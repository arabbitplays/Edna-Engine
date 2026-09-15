#ifndef EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP

#include <chrono>

#include <library/animation/animations/IAnimation.hpp>
#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationGenerator.hpp>
#include <library/cellular_automaton/colors/ColorPalette.hpp>

namespace cellular_automaton
{
    // Owns the two CCA animation tracks (mutation chance + palette). Each track
    // advances independently: when its current animation finishes, a random
    // cooldown in [COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS] elapses before the
    // next animation is generated and chained from the value the previous one
    // ended on.
    class CyclicalCellularAutomatonAnimationRunner
    {
    public:
        static constexpr float COOLDOWN_MIN_SECONDS = 10.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 30.0f;

        CyclicalCellularAutomatonAnimationRunner(
            CyclicalCellularAutomatonAnimationGenerator generator,
            float initial_mutation_chance,
            ColorPalette initial_palette);

        void update();

    private:
        struct Track
        {
            ::Animation::AnimationHandle animation;
            float cooldown_seconds = 0.0f;
        };

        void tick(Track& track, float dt, void (CyclicalCellularAutomatonAnimationRunner::*start)());
        void startMutation();
        void startPalette();

        CyclicalCellularAutomatonAnimationGenerator generator_;

        Track        mutation_track_;
        float        mutation_current_;

        Track        palette_track_;
        ColorPalette palette_current_;

        std::chrono::steady_clock::time_point last_tick_;
    };
}

#endif //EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP
