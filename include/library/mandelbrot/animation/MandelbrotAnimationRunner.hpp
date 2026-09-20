#ifndef EDNA_ENGINE_MANDELBROT_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_MANDELBROT_ANIMATIONRUNNER_HPP

#include <chrono>

#include <glm/vec2.hpp>

#include <library/animation/animations/IAnimation.hpp>
#include <library/color/ColorPalette.hpp>
#include <library/mandelbrot/animation/MandelbrotAnimationGenerator.hpp>

namespace mandelbrot
{
    // Owns four Mandelbrot animation tracks (offset, step_size, initial_number,
    // palette). Each track advances independently: when its current animation
    // finishes, a cooldown elapses before the next animation is generated,
    // chained from the value the previous one ended on.
    //
    // The runner accepts a per-update entropy metric from the renderer and
    // uses it to derive a global speed multiplier: high entropy (interesting
    // frames) slows everything down so the view lingers; low entropy
    // (uninteresting frames) speeds it up so we escape faster.
    class MandelbrotAnimationRunner
    {
    public:
        // Cooldown between animations, measured in "virtual" seconds — real
        // wall-clock cooldown is longer when speed_multiplier < 1.
        static constexpr float COOLDOWN_MIN_SECONDS = 2.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 6.0f;

        // Speed multiplier bounds. 1.0 = one animation step per rendered frame.
        static constexpr float MIN_SPEED = 0.2f;   // reached when entropy saturates
        static constexpr float MAX_SPEED = 4.0f;   // reached when entropy ~= 0

        // Time constant (seconds) for the exponential smoothing of the speed
        // multiplier. Larger values ramp speed changes in and out more slowly
        // — with 1.5s the multiplier reaches ~63% of a step change over 1.5s
        // rather than jumping every frame with the raw entropy reading.
        static constexpr float SPEED_SMOOTHING_TAU_SECONDS = 1.5f;

        MandelbrotAnimationRunner(
            MandelbrotAnimationGenerator generator,
            glm::vec2              initial_offset,
            float                  initial_step_size,
            glm::vec2              initial_initial_number,
            ::color::ColorPalette  initial_palette);

        // Advances all four tracks. entropy_bits should be the value returned
        // by MandelbrotRenderer::readEntropy(); max_entropy_bits is
        // log2(HISTOGRAM_BIN_COUNT). Passing max_entropy_bits <= 0 disables
        // speed modulation (multiplier = 1).
        void update(float entropy_bits, float max_entropy_bits);

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

        MandelbrotAnimationGenerator generator_;

        Track offset_track_;
        Track step_track_;
        Track initial_track_;
        Track palette_track_;

        glm::vec2             offset_current_;
        float                 step_size_current_;
        glm::vec2             initial_current_;
        ::color::ColorPalette palette_current_;

        float smoothed_speed_ = 1.0f;

        std::chrono::steady_clock::time_point last_tick_;
    };
}

#endif //EDNA_ENGINE_MANDELBROT_ANIMATIONRUNNER_HPP
