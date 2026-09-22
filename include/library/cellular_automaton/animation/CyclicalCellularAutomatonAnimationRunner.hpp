#ifndef EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP

#include <chrono>
#include <library/animation/animations/IAnimation.hpp>
#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationGenerator.hpp>

namespace cellular_automaton
{
    // Drives the CCA mutation-chance animation on a random cooldown in
    // [COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS] and periodically re-rolls
    // the neighborhood. Palette animation is owned centrally by the
    // composition manager.
    class CyclicalCellularAutomatonAnimationRunner
    {
    public:
        static constexpr float COOLDOWN_MIN_SECONDS = 10.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 30.0f;

        CyclicalCellularAutomatonAnimationRunner(CyclicalCellularAutomatonAnimationGenerator generator,
            float initial_mutation_chance);

        void update();

    private:
        struct Track
        {
            ::Animation::AnimationHandle animation;
            float cooldown_seconds = 0.0f;
        };

        void tick(Track& track, float dt, void (CyclicalCellularAutomatonAnimationRunner::*start)());
        void startMutation();

        CyclicalCellularAutomatonAnimationGenerator generator_;

        Track mutation_track_;
        float mutation_current_;

        float neighborhood_cooldown_seconds_ = 0.0f;

        std::chrono::steady_clock::time_point last_tick_;
    };
} // namespace cellular_automaton

#endif // EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP
