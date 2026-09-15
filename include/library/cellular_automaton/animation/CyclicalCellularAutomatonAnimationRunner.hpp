#ifndef EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP

#include <vector>

#include <library/animation/animations/IAnimation.hpp>
#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationGenerator.hpp>
#include <library/cellular_automaton/colors/ColorPalette.hpp>

namespace cellular_automaton
{
    // Owns the currently-running CCA animations and drives them forward. When
    // every held animation reports finished(), a fresh batch is requested from
    // the generator, chained from the values the previous batch ended on.
    class CyclicalCellularAutomatonAnimationRunner
    {
    public:
        CyclicalCellularAutomatonAnimationRunner(
            CyclicalCellularAutomatonAnimationGenerator generator,
            float initial_mutation_chance,
            ColorPalette initial_palette);

        void update();

    private:
        void regenerate();
        bool allFinished() const;

        CyclicalCellularAutomatonAnimationGenerator generator_;
        std::vector<::Animation::AnimationHandle> animations_;

        float current_mutation_chance_;
        ColorPalette current_palette_;
    };
}

#endif //EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP
