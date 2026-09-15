#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationGenerator.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>
#include <library/cellular_automaton/colors/ColorPaletteFactory.hpp>
#include <library/cellular_automaton/colors/ColorPaletteName.hpp>
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
        std::function<void(const ColorPalette&)> set_palette)
        : set_mutation_chance_(std::move(set_mutation_chance)),
          set_palette_(std::move(set_palette))
    {
    }

    CyclicalCellularAutomatonAnimationGenerator::MutationChanceAnimation
    CyclicalCellularAutomatonAnimationGenerator::generateMutationChanceAnimation(float current)
    {
        const float target = randomFloat(MUTATION_CHANCE_MIN, MUTATION_CHANCE_MAX);

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

    ColorPalette CyclicalCellularAutomatonAnimationGenerator::pickRandomPalette()
    {
        const auto all_names = ColorPaletteName::getAllNames();
        const std::size_t idx = RtEngine::RandomUtil::generateInt() % all_names.size();
        const auto name = ColorPaletteName::fromString(all_names[idx], ColorPaletteName::Fire);
        return ColorPaletteFactory::create(name);
    }
}
