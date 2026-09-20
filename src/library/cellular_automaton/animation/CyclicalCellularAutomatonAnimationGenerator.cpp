#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationGenerator.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

#include <spdlog/spdlog.h>

#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>
#include <library/cellular_automaton/colors/ColorPaletteFactory.hpp>
#include <library/cellular_automaton/colors/ColorPaletteName.hpp>
#include <library/cellular_automaton/neighborhoods/NeighborhoodFactory.hpp>
#include <library/cellular_automaton/neighborhoods/NeighborhoodShape.hpp>
#include <util/RandomUtil.hpp>

namespace cellular_automaton
{
    namespace
    {
        float randomFloat(float min, float max)
        {
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return min + t * (max - min);
        }

        int randomStepCount()
        {
            using Gen = CyclicalCellularAutomatonAnimationGenerator;
            const auto range = static_cast<uint32_t>(Gen::STEP_COUNT_MAX - Gen::STEP_COUNT_MIN + 1);
            return Gen::STEP_COUNT_MIN + static_cast<int>(RtEngine::RandomUtil::generateInt() % range);
        }

        std::shared_ptr<::Animation::EasingFunction> randomInOutEasing()
        {
            constexpr std::array curves = {
                ::Animation::EasingCurve::Linear,
                ::Animation::EasingCurve::Cubic,
                ::Animation::EasingCurve::Elastic,
                ::Animation::EasingCurve::Bounce,
            };
            const std::size_t idx = RtEngine::RandomUtil::generateInt() % curves.size();
            return ::Animation::makeEasingFunction(curves[idx], ::Animation::EasingDirection::InOut);
        }
    }

    CyclicalCellularAutomatonAnimationGenerator::CyclicalCellularAutomatonAnimationGenerator(
        std::function<void(float)> set_mutation_chance,
        std::function<void(const ColorPalette&)> set_palette,
        std::function<void(const std::vector<glm::ivec2>&)> set_neighborhood,
        std::function<void(uint32_t)> set_threshold)
        : set_mutation_chance_(std::move(set_mutation_chance)),
          set_palette_(std::move(set_palette)),
          set_neighborhood_(std::move(set_neighborhood)),
          set_threshold_(std::move(set_threshold))
    {
    }

    CyclicalCellularAutomatonAnimationGenerator::MutationChanceAnimation
    CyclicalCellularAutomatonAnimationGenerator::generateMutationChanceAnimation(float current)
    {
        const float target = pickRandomMutationChance();

        auto set = set_mutation_chance_;
        auto animation = std::make_unique<::Animation::FloatAnimation>(
            current,
            target,
            randomStepCount(),
            [set](const float& v) { if (set) set(v); },
            randomInOutEasing());

        return {std::move(animation), target};
    }

    CyclicalCellularAutomatonAnimationGenerator::PaletteAnimation
    CyclicalCellularAutomatonAnimationGenerator::generatePaletteAnimation(const ColorPalette& current)
    {
        ColorPalette target = pickRandomPalette();

        auto set = set_palette_;
        auto animation = std::make_unique<ColorPaletteAnimation>(
            current,
            target,
            randomStepCount(),
            [set](const ColorPalette& p) { if (set) set(p); },
            randomInOutEasing());

        return {std::move(animation), std::move(target)};
    }

    void CyclicalCellularAutomatonAnimationGenerator::applyRandomNeighborhood()
    {
        auto pick = pickRandomNeighborhood();

        if (set_neighborhood_) set_neighborhood_(pick.offsets);
        if (set_threshold_)    set_threshold_(pick.threshold);
    }

    float CyclicalCellularAutomatonAnimationGenerator::pickRandomMutationChance()
    {
        const float target = randomFloat(MUTATION_CHANCE_MIN, MUTATION_CHANCE_MAX);
        spdlog::info("CCA anim: rolled mutation_chance target = {:.4f}", target);
        return target;
    }

    ColorPalette CyclicalCellularAutomatonAnimationGenerator::pickRandomPalette()
    {
        const auto all_names = ColorPaletteName::getAllNames();
        const std::size_t idx = RtEngine::RandomUtil::generateInt() % all_names.size();
        const auto& picked_name = all_names[idx];
        spdlog::info("CCA anim: rolled palette target = {}", picked_name);
        return ColorPaletteFactory::create(
            ColorPaletteName::fromString(picked_name, ColorPaletteName::Fire));
    }

    CyclicalCellularAutomatonAnimationGenerator::NeighborhoodPick
    CyclicalCellularAutomatonAnimationGenerator::pickRandomNeighborhood()
    {
        constexpr std::array<uint32_t, 3> sizes = {1u, 2u, 3u};
        const uint32_t size = sizes[RtEngine::RandomUtil::generateInt() % sizes.size()];

        const auto shape_names = NeighborhoodShape::getAllNames();
        const std::size_t shape_idx = RtEngine::RandomUtil::generateInt() % shape_names.size();
        const auto shape = NeighborhoodShape::fromString(shape_names[shape_idx], NeighborhoodShape::Box);

        uint32_t threshold = 0u;
        if (size == 1u) {
            threshold = 1u;
        } else {
            switch (shape) {
                case NeighborhoodShape::Box:     threshold = size * 2u; break;
                case NeighborhoodShape::Diamond: threshold = size;      break;
            }
        }

        spdlog::info("CCA anim: rolled neighborhood shape={}, size={}, threshold={}",
                     shape_names[shape_idx], size, threshold);

        return {NeighborhoodFactory::create(shape, static_cast<int>(size)).offsets, threshold};
    }
}
