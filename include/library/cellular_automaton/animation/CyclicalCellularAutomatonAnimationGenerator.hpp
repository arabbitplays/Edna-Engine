#ifndef EDNA_ENGINE_CCA_ANIMATIONGENERATOR_HPP
#define EDNA_ENGINE_CCA_ANIMATIONGENERATOR_HPP

#include <functional>
#include <memory>

#include <library/animation/animations/FloatAnimation.hpp>
#include <library/cellular_automaton/colors/ColorPalette.hpp>
#include <library/cellular_automaton/colors/ColorPaletteAnimation.hpp>

namespace cellular_automaton
{
    class CyclicalCellularAutomatonAnimationGenerator
    {
    public:
        static constexpr int   STEP_COUNT_MIN       = 2000;
        static constexpr int   STEP_COUNT_MAX       = 10000;
        static constexpr float MUTATION_CHANCE_MIN  = 0.0f;
        static constexpr float MUTATION_CHANCE_MAX  = 0.1f;

        CyclicalCellularAutomatonAnimationGenerator(
            std::function<void(float)> set_mutation_chance,
            std::function<void(const ColorPalette&)> set_palette);

        struct MutationChanceAnimation
        {
            std::unique_ptr<::Animation::FloatAnimation> animation;
            float target;
        };

        struct PaletteAnimation
        {
            std::unique_ptr<ColorPaletteAnimation> animation;
            ColorPalette target;
        };

        MutationChanceAnimation generateMutationChanceAnimation(float current);
        PaletteAnimation        generatePaletteAnimation(const ColorPalette& current);

    private:
        ColorPalette pickRandomPalette();

        std::function<void(float)> set_mutation_chance_;
        std::function<void(const ColorPalette&)> set_palette_;
    };
}

#endif //EDNA_ENGINE_CCA_ANIMATIONGENERATOR_HPP
