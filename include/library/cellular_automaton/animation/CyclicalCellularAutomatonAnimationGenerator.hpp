#ifndef EDNA_ENGINE_CCA_ANIMATIONGENERATOR_HPP
#define EDNA_ENGINE_CCA_ANIMATIONGENERATOR_HPP

#include <cstdint>
#include <functional>
#include <glm/vec2.hpp>
#include <library/animation/animations/FloatAnimation.hpp>
#include <library/color/ColorPalette.hpp>
#include <library/color/ColorPaletteAnimation.hpp>
#include <memory>
#include <vector>

namespace cellular_automaton
{
    class CyclicalCellularAutomatonAnimationGenerator
    {
    public:
        static constexpr int STEP_COUNT_MIN = 2000;
        static constexpr int STEP_COUNT_MAX = 10000;
        static constexpr float MUTATION_CHANCE_MIN = 0.0f;
        static constexpr float MUTATION_CHANCE_MAX = 0.1f;

        CyclicalCellularAutomatonAnimationGenerator(std::function<void(float)> set_mutation_chance,
            std::function<void(const ::color::ColorPalette&)> set_palette,
            std::function<void(const std::vector<glm::ivec2>&)> set_neighborhood,
            std::function<void(uint32_t)> set_threshold);

        struct MutationChanceAnimation
        {
            std::unique_ptr<::Animation::FloatAnimation> animation;
            float target;
        };

        struct PaletteAnimation
        {
            std::unique_ptr<::color::ColorPaletteAnimation> animation;
            ::color::ColorPalette target;
        };

        MutationChanceAnimation generateMutationChanceAnimation(float current);
        PaletteAnimation generatePaletteAnimation(const ::color::ColorPalette& current);

        // Neighborhood + threshold are re-rolled together, not interpolated.
        void applyRandomNeighborhood();

    private:
        struct NeighborhoodPick
        {
            std::vector<glm::ivec2> offsets;
            uint32_t threshold;
        };

        static float pickRandomMutationChance();
        static ::color::ColorPalette pickRandomPalette();
        static NeighborhoodPick pickRandomNeighborhood();

        std::function<void(float)> set_mutation_chance_;
        std::function<void(const ::color::ColorPalette&)> set_palette_;
        std::function<void(const std::vector<glm::ivec2>&)> set_neighborhood_;
        std::function<void(uint32_t)> set_threshold_;
    };
} // namespace cellular_automaton

#endif // EDNA_ENGINE_CCA_ANIMATIONGENERATOR_HPP
