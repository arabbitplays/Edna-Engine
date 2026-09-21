#ifndef EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP
#define EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP

#include <cstdint>
#include <functional>
#include <memory>

#include <glm/vec2.hpp>

#include <library/animation/animations/FloatAnimation.hpp>
#include <library/animation/animations/VectorAnimation.hpp>
#include <library/color/ColorPalette.hpp>
#include <library/color/ColorPaletteAnimation.hpp>
#include <library/mandelbrot/MandelbrotState.hpp>

namespace mandelbrot
{
    class MandelbrotAnimationGenerator
    {
    public:
        static constexpr int   STEP_COUNT_MIN     = 800;
        static constexpr int   STEP_COUNT_MAX     = 4000;

        static constexpr float OFFSET_MIN         = -1.0f;
        static constexpr float OFFSET_MAX         =  1.0f;

        // step_size animated in log10 space so linear interpolation covers
        // orders of magnitude evenly.
        static constexpr float LOG_STEP_SIZE_MIN  = -6.0f;
        static constexpr float LOG_STEP_SIZE_MAX  = -3.0f;

        static constexpr float INITIAL_MIN        = -1.2f;
        static constexpr float INITIAL_MAX        =  1.2f;

        // Reject candidates whose probe scores below PROBE_ACCEPT_EDGE or
        // whose inside_fraction exceeds PROBE_MAX_INSIDE_FRACT. If no
        // attempt clears the gate, the best-scoring candidate wins so
        // generation never blocks.
        static constexpr float         PROBE_ACCEPT_EDGE       = 0.05f;
        static constexpr float         PROBE_MAX_INSIDE_FRACT  = 0.60f;
        static constexpr std::uint32_t PROBE_MAX_ATTEMPTS      = 8u;
        static constexpr std::uint32_t PROBE_GRID_SIZE         = 16u;
        static constexpr std::uint32_t PROBE_MAX_ITER          = 256u;

        // Canonical span used when the generator can't see the actual
        // on-screen span. Targets that score well here are the
        // "structurally interesting" ones.
        static constexpr float         PROBE_REFERENCE_SPAN     = 3.0f;

        MandelbrotAnimationGenerator(
            std::function<void(const glm::vec2&)>             set_offset,
            std::function<void(float)>                        set_step_size,
            std::function<void(const glm::vec2&)>             set_initial,
            std::function<void(const ::color::ColorPalette&)> set_palette);

        struct Vec2AnimationResult
        {
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

        // Candidates are scored against `current` for the fields the method
        // does not roll, so target selection sees the actual fractal being
        // rendered (in particular the current initial/c and julia_mode).
        Vec2AnimationResult    generateOffsetAnimation(const MandelbrotState& current);
        FloatAnimationResult   generateStepSizeAnimation(const MandelbrotState& current);
        Vec2AnimationResult    generateInitialAnimation(const MandelbrotState& current);
        PaletteAnimationResult generatePaletteAnimation(const ::color::ColorPalette& current);

    private:
        std::function<void(const glm::vec2&)> set_offset_;
        std::function<void(float)> set_step_size_;
        std::function<void(const glm::vec2&)> set_initial_;
        std::function<void(const ::color::ColorPalette&)> set_palette_;
    };
}

#endif //EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP
