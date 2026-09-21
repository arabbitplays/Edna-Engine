#ifndef EDNA_ENGINE_MANDELBROT_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_MANDELBROT_ANIMATIONRUNNER_HPP

#include <chrono>

#include <glm/vec2.hpp>

#include <library/animation/animations/IAnimation.hpp>
#include <library/color/ColorPalette.hpp>
#include <library/mandelbrot/MandelbrotState.hpp>
#include <library/mandelbrot/animation/MandelbrotAnimationGenerator.hpp>

namespace mandelbrot
{
    // Drives four independent Mandelbrot animation tracks (offset, step_size,
    // initial, palette). Each track chains to a fresh animation after a
    // random cooldown, using the value the previous animation ended on.
    class MandelbrotAnimationRunner
    {
    public:
        // Cooldown ticks in virtual seconds; real wall time is longer when
        // speed_multiplier < 1.
        static constexpr float COOLDOWN_MIN_SECONDS = 2.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 6.0f;

        // 1.0 = one animation step per rendered frame.
        static constexpr float MIN_SPEED = 0.2f;   // at high entropy
        static constexpr float MAX_SPEED = 4.0f;   // at zero entropy

        // EMA time constant on the speed multiplier so entropy spikes don't
        // translate to jarring per-frame speed changes.
        static constexpr float SPEED_SMOOTHING_TAU_SECONDS = 1.5f;

        MandelbrotAnimationRunner(
            std::function<void(const glm::vec2&)>             set_offset,
            std::function<void(float)>                        set_step_size,
            std::function<void(const glm::vec2&)>             set_initial,
            std::function<void(const ::color::ColorPalette&)> set_palette,
            MandelbrotState        initial_state,
            ::color::ColorPalette  initial_palette);

        // max_entropy_bits <= 0 disables speed modulation.
        void update(float entropy_bits, float max_entropy_bits, bool julia_mode);

    private:
        struct Track
        {
            ::Animation::AnimationHandle animation;
            float cooldown_seconds = 0.0f;
            float step_accumulator = 0.0f;
        };

        float computeSpeed(float entropy_bits, float max_entropy_bits) const;
        void tick(Track& track, float dt, float speed,
                  void (MandelbrotAnimationRunner::*start)());

        void startOffset();
        void startStepSize();
        void startInitial();
        void startPalette();

        // Declared before generator_ so the setter wrappers below see a
        // fully-initialised target (init order = declaration order).
        MandelbrotState       current_state_{};
        ::color::ColorPalette palette_current_;

        MandelbrotAnimationGenerator generator_;

        Track offset_track_;
        Track step_track_;
        Track initial_track_;
        Track palette_track_;

        float smoothed_speed_ = 1.0f;

        std::chrono::steady_clock::time_point last_tick_;
    };
}

#endif //EDNA_ENGINE_MANDELBROT_ANIMATIONRUNNER_HPP
