#include <array>
#include <cstdint>
#include <format>
#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>
#include <library/color/ColorPaletteAnimationGenerator.hpp>
#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>
#include <logging/LogManager.hpp>
#include <memory>
#include <util/RandomUtil.hpp>
#include <utility>

namespace color
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<ColorPaletteAnimationGenerator>();
            return instance;
        }

        int randomStepCount()
        {
            using Gen = ColorPaletteAnimationGenerator;
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

        ColorPaletteName pickRandomPaletteName()
        {
            const auto all_names = ColorPaletteName::getAllNames();
            const std::size_t idx = RtEngine::RandomUtil::generateInt() % all_names.size();
            return ColorPaletteName::fromString(all_names[idx], ColorPaletteName::Fire);
        }
    } // namespace

    ColorPaletteAnimationGenerator::ColorPaletteAnimationGenerator(std::function<void(const ColorPalette&)> set_palette)
        : set_palette_(std::move(set_palette))
    {
    }

    ColorPaletteAnimationGenerator::PaletteAnimationResult
    ColorPaletteAnimationGenerator::generatePaletteAnimation(const ColorPalette& current)
    {
        const ColorPaletteName target_name = pickRandomPaletteName();
        ColorPalette target = ColorPaletteFactory::create(target_name);
        logger()->debug(std::format("rolled palette target = {}", target_name.toString()));

        auto set = set_palette_;
        auto animation = std::make_unique<ColorPaletteAnimation>(
            current, target, randomStepCount(),
            [set](const ColorPalette& p)
            {
                if (set)
                {
                    set(p);
                }
            },
            randomInOutEasing());

        return {.animation = std::move(animation), .target = std::move(target)};
    }
} // namespace color
