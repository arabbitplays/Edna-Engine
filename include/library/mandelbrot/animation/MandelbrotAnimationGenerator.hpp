#ifndef EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP
#define EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP

#include <functional>
#include <memory>

#include <glm/vec2.hpp>

#include <library/animation/animations/FloatAnimation.hpp>
#include <library/animation/animations/VectorAnimation.hpp>
#include <library/color/ColorPalette.hpp>
#include <library/color/ColorPaletteAnimation.hpp>

namespace mandelbrot
{
    class MandelbrotAnimationGenerator
    {
    public:
        static constexpr int   STEP_COUNT_MIN     = 800;
        static constexpr int   STEP_COUNT_MAX     = 4000;

        // Offset is in screen units (same coordinate space as the component).
        static constexpr float OFFSET_MIN         = -1.0f;
        static constexpr float OFFSET_MAX         =  1.0f;

        // step_size is animated in log10 space so linear interpolation covers
        // multiple orders of magnitude without lingering near the small end.
        static constexpr float LOG_STEP_SIZE_MIN  = -4.0f; // 1e-3
        static constexpr float LOG_STEP_SIZE_MAX  = -3.0f; // 1e-2

        // initial_number kept modest so we mostly get recognisable
        // Julia-set shapes and slightly-perturbed Mandelbrots.
        static constexpr float INITIAL_MIN        = -1.2f;
        static constexpr float INITIAL_MAX        =  1.2f;

        MandelbrotAnimationGenerator(
            std::function<void(const glm::vec2&)>          set_offset,
            std::function<void(float)>                     set_step_size,
            std::function<void(const glm::vec2&)>          set_initial,
            std::function<void(const ::color::ColorPalette&)> set_palette);

        struct Vec2AnimationResult
        {
            // Base type so callers can hold either a straight-line lerp or a
            // Bezier without knowing which one was produced.
            std::unique_ptr<::Animation::Animation<glm::vec2>> animation;
            glm::vec2 target;
        };

        struct FloatAnimationResult
        {
            std::unique_ptr<::Animation::FloatAnimation> animation;
            float target;
        };

        struct PaletteAnimationResult
        {
            std::unique_ptr<::color::ColorPaletteAnimation> animation;
            ::color::ColorPalette target;
        };

        Vec2AnimationResult    generateOffsetAnimation(const glm::vec2& current);
        FloatAnimationResult   generateStepSizeAnimation(float current);
        Vec2AnimationResult    generateInitialAnimation(const glm::vec2& current);
        PaletteAnimationResult generatePaletteAnimation(const ::color::ColorPalette& current);

    private:
        std::function<void(const glm::vec2&)> set_offset_;
        std::function<void(float)> set_step_size_;
        std::function<void(const glm::vec2&)> set_initial_;
        std::function<void(const ::color::ColorPalette&)> set_palette_;
    };
}

#endif //EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP
