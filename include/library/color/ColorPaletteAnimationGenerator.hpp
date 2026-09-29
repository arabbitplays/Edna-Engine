#ifndef EDNA_ENGINE_COLORPALETTEANIMATIONGENERATOR_HPP
#define EDNA_ENGINE_COLORPALETTEANIMATIONGENERATOR_HPP

#include <functional>
#include <library/color/ColorPalette.hpp>
#include <library/color/ColorPaletteAnimation.hpp>
#include <memory>

namespace color
{
    // Rolls a random palette target and produces a ColorPaletteAnimation that
    // interpolates toward it, dispatching each interpolated step through the
    // caller-provided setter.
    class ColorPaletteAnimationGenerator
    {
    public:
        // Runner ticks the animation on a per-frame gate; this range yields
        // a smooth transition of a few seconds.
        static constexpr int STEP_COUNT_MIN = 2000;
        static constexpr int STEP_COUNT_MAX = 10000;

        struct PaletteAnimationResult
        {
            std::unique_ptr<ColorPaletteAnimation> animation;
            ColorPalette target;
        };

        explicit ColorPaletteAnimationGenerator(std::function<void(const ColorPalette&)> set_palette);

        PaletteAnimationResult generatePaletteAnimation(const ColorPalette& current);

    private:
        std::function<void(const ColorPalette&)> set_palette_;
    };
} // namespace color

#endif // EDNA_ENGINE_COLORPALETTEANIMATIONGENERATOR_HPP
