#include <array>
#include <cstdint>
#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>
#include <library/color/ColorPaletteAnimation.hpp>
#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>
#include <library/color/PaletteAnimationRunner.hpp>
#include <limits>
#include <memory>
#include <util/RandomUtil.hpp>
#include <utility>

namespace color
{
    namespace
    {
        constexpr int STEP_COUNT_MIN = 2000;
        constexpr int STEP_COUNT_MAX = 10000;

        float randomFloat(float min, float max)
        {
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return min + (t * (max - min));
        }

        float randomCooldown()
        {
            return randomFloat(PaletteAnimationRunner::COOLDOWN_MIN_SECONDS,
                PaletteAnimationRunner::COOLDOWN_MAX_SECONDS);
        }

        int randomStepCount()
        {
            const auto range = static_cast<uint32_t>(STEP_COUNT_MAX - STEP_COUNT_MIN + 1);
            return STEP_COUNT_MIN + static_cast<int>(RtEngine::RandomUtil::generateInt() % range);
        }

        std::shared_ptr<::Animation::EasingFunction> randomInOutEasing()
        {
            constexpr std::array curves = {
                ::Animation::EasingCurve::Linear,
                ::Animation::EasingCurve::Cubic,
                ::Animation::EasingCurve::Elastic,
            };
            const std::size_t idx = RtEngine::RandomUtil::generateInt() % curves.size();
            return ::Animation::makeEasingFunction(curves[idx], ::Animation::EasingDirection::InOut);
        }

        ColorPalette pickRandomPalette()
        {
            const auto all_names = ColorPaletteName::getAllNames();
            const std::size_t idx = RtEngine::RandomUtil::generateInt() % all_names.size();
            return ColorPaletteFactory::create(ColorPaletteName::fromString(all_names[idx], ColorPaletteName::Fire));
        }
    } // namespace

    PaletteAnimationRunner::PaletteAnimationRunner(ColorPalette initial_palette)
        : current_palette_(std::move(initial_palette)), last_tick_(std::chrono::steady_clock::now())
    {
        startAnimation();
    }

    void PaletteAnimationRunner::addListener(Listener listener)
    {
        if (!listener)
        {
            return;
        }
        listener(current_palette_);
        listeners_.push_back(std::move(listener));
    }

    void PaletteAnimationRunner::update()
    {
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last_tick_).count();
        last_tick_ = now;

        if (animation_)
        {
            animation_->step();
            if (animation_->finished())
            {
                animation_.reset();
                cooldown_seconds_ = randomCooldown();
            }
            return;
        }

        cooldown_seconds_ -= dt;
        if (cooldown_seconds_ <= 0.0F)
        {
            cooldown_seconds_ = 0.0F;
            startAnimation();
        }
    }

    void PaletteAnimationRunner::broadcast(const ColorPalette& palette)
    {
        current_palette_ = palette;
        for (const auto& listener : listeners_)
        {
            listener(palette);
        }
    }

    void PaletteAnimationRunner::startAnimation()
    {
        ColorPalette target = pickRandomPalette();

        animation_ = std::make_shared<ColorPaletteAnimation>(
            current_palette_, target, randomStepCount(),
            [this](const ColorPalette& p) { broadcast(p); },
            randomInOutEasing());
    }
} // namespace color
