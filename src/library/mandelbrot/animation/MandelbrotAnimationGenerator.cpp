#include <library/mandelbrot/animation/MandelbrotAnimationGenerator.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

#include <spdlog/spdlog.h>

#include <library/animation/animations/BezierAnimation.hpp>
#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>
#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>
#include <util/RandomUtil.hpp>

namespace mandelbrot
{
    namespace
    {
        float randomFloat(float min, float max)
        {
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return min + t * (max - min);
        }

        glm::vec2 randomVec2(float min, float max)
        {
            return {randomFloat(min, max), randomFloat(min, max)};
        }

        int randomStepCount()
        {
            using Gen = MandelbrotAnimationGenerator;
            const auto range = static_cast<uint32_t>(Gen::STEP_COUNT_MAX - Gen::STEP_COUNT_MIN + 1);
            return Gen::STEP_COUNT_MIN + static_cast<int>(RtEngine::RandomUtil::generateInt() % range);
        }

        std::shared_ptr<::Animation::EasingFunction> randomInOutEasing()
        {
            constexpr std::array curves = {
                ::Animation::EasingCurve::Cubic
            };
            const std::size_t idx = RtEngine::RandomUtil::generateInt() % curves.size();
            return ::Animation::makeEasingFunction(curves[idx], ::Animation::EasingDirection::InOut);
        }
    }

    MandelbrotAnimationGenerator::MandelbrotAnimationGenerator(
        std::function<void(const glm::vec2&)>             set_offset,
        std::function<void(float)>                        set_step_size,
        std::function<void(const glm::vec2&)>             set_initial,
        std::function<void(const ::color::ColorPalette&)> set_palette)
        : set_offset_(std::move(set_offset)),
          set_step_size_(std::move(set_step_size)),
          set_initial_(std::move(set_initial)),
          set_palette_(std::move(set_palette))
    {
    }

    MandelbrotAnimationGenerator::Vec2AnimationResult
    MandelbrotAnimationGenerator::generateOffsetAnimation(const glm::vec2& current)
    {
        // Precompute 3 offsets: two intermediate Bezier control points shape
        // the middle of the curve, and the third is the destination the curve
        // ends at. The result is a smoother arc through the offset space than
        // a straight lerp.
        const glm::vec2 c1     = randomVec2(OFFSET_MIN, OFFSET_MAX);
        const glm::vec2 c2     = randomVec2(OFFSET_MIN, OFFSET_MAX);
        const glm::vec2 target = randomVec2(OFFSET_MIN, OFFSET_MAX);
        spdlog::info("Mandelbrot anim: offset bezier c1=({:.3f},{:.3f}) c2=({:.3f},{:.3f}) target=({:.3f},{:.3f})",
                     c1.x, c1.y, c2.x, c2.y, target.x, target.y);

        auto set = set_offset_;
        auto animation = std::make_unique<::Animation::Vec2BezierAnimation>(
            current, c1, c2, target, randomStepCount(),
            [set](const glm::vec2& v) { if (set) set(v); },
            randomInOutEasing());

        return {std::move(animation), target};
    }

    MandelbrotAnimationGenerator::FloatAnimationResult
    MandelbrotAnimationGenerator::generateStepSizeAnimation(float current)
    {
        const float log_target = randomFloat(LOG_STEP_SIZE_MIN, LOG_STEP_SIZE_MAX);
        const float target = std::pow(10.0f, log_target);
        // Interpolate the log so the visual zoom speed stays roughly uniform.
        const float log_current = std::log10(std::max(current, 1e-9f));
        spdlog::info("Mandelbrot anim: step_size target = {:.6f} (log={:.3f})", target, log_target);

        auto set = set_step_size_;
        auto animation = std::make_unique<::Animation::FloatAnimation>(
            log_current, log_target, randomStepCount(),
            [set](const float& log_v) { if (set) set(std::pow(10.0f, log_v)); },
            randomInOutEasing());

        return {std::move(animation), target};
    }

    MandelbrotAnimationGenerator::Vec2AnimationResult
    MandelbrotAnimationGenerator::generateInitialAnimation(const glm::vec2& current)
    {
        const glm::vec2 target = randomVec2(INITIAL_MIN, INITIAL_MAX);
        spdlog::info("Mandelbrot anim: initial target = ({:.3f}, {:.3f})", target.x, target.y);

        auto set = set_initial_;
        auto animation = std::make_unique<::Animation::Vec2Animation>(
            current, target, randomStepCount(),
            [set](const glm::vec2& v) { if (set) set(v); },
            randomInOutEasing());

        return {std::move(animation), target};
    }

    MandelbrotAnimationGenerator::PaletteAnimationResult
    MandelbrotAnimationGenerator::generatePaletteAnimation(const ::color::ColorPalette& current)
    {
        const auto all_names = ::color::ColorPaletteName::getAllNames();
        const std::size_t idx = RtEngine::RandomUtil::generateInt() % all_names.size();
        const auto& picked_name = all_names[idx];
        spdlog::info("Mandelbrot anim: palette target = {}", picked_name);

        ::color::ColorPalette target = ::color::ColorPaletteFactory::create(
            ::color::ColorPaletteName::fromString(picked_name, ::color::ColorPaletteName::Fire));

        auto set = set_palette_;
        auto animation = std::make_unique<::color::ColorPaletteAnimation>(
            current, target, randomStepCount(),
            [set](const ::color::ColorPalette& p) { if (set) set(p); },
            randomInOutEasing());

        return {std::move(animation), std::move(target)};
    }
}
