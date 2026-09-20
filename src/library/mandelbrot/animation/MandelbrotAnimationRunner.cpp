#include <library/mandelbrot/animation/MandelbrotAnimationRunner.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

#include <util/RandomUtil.hpp>

namespace mandelbrot
{
    namespace
    {
        float randomCooldown()
        {
            using Runner = MandelbrotAnimationRunner;
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return Runner::COOLDOWN_MIN_SECONDS +
                   t * (Runner::COOLDOWN_MAX_SECONDS - Runner::COOLDOWN_MIN_SECONDS);
        }
    }

    MandelbrotAnimationRunner::MandelbrotAnimationRunner(
        MandelbrotAnimationGenerator generator,
        glm::vec2             initial_offset,
        float                 initial_step_size,
        glm::vec2             initial_initial_number,
        ::color::ColorPalette initial_palette)
        : generator_(std::move(generator)),
          offset_current_(initial_offset),
          step_size_current_(initial_step_size),
          initial_current_(initial_initial_number),
          palette_current_(std::move(initial_palette)),
          last_tick_(std::chrono::steady_clock::now())
    {
        startOffset();
        startStepSize();
        startInitial();
        startPalette();
    }

    void MandelbrotAnimationRunner::update(float entropy_bits, float max_entropy_bits)
    {
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last_tick_).count();
        last_tick_ = now;

        // Exponential moving average on the speed multiplier so entropy
        // spikes don't translate to jarring per-frame speed changes. The
        // blend factor is frame-rate independent: with dt ≈ 1/60s and TAU
        // = 1.5s, alpha is ~0.011 per frame, giving a smooth ramp over
        // roughly a second.
        const float target_speed = computeSpeed(entropy_bits, max_entropy_bits);
        const float alpha = 1.0f - std::exp(-dt / SPEED_SMOOTHING_TAU_SECONDS);
        smoothed_speed_ += (target_speed - smoothed_speed_) * alpha;

        tick(offset_track_,  dt, smoothed_speed_, &MandelbrotAnimationRunner::startOffset);
        tick(step_track_,    dt, smoothed_speed_, &MandelbrotAnimationRunner::startStepSize);
        tick(initial_track_, dt, smoothed_speed_, &MandelbrotAnimationRunner::startInitial);
        tick(palette_track_, dt, smoothed_speed_, &MandelbrotAnimationRunner::startPalette);
    }

    float MandelbrotAnimationRunner::computeSpeed(float entropy_bits, float max_entropy_bits) const
    {
        if (max_entropy_bits <= 0.0f) return 1.0f;
        const float x = std::clamp(entropy_bits / max_entropy_bits, 0.0f, 1.0f);
        // Linear map: entropy = 0  -> MAX_SPEED (find interesting frame fast)
        //             entropy = 1  -> MIN_SPEED (linger on this frame)
        return MAX_SPEED - (MAX_SPEED - MIN_SPEED) * x;
    }

    void MandelbrotAnimationRunner::tick(
        Track& track, float dt, float speed,
        void (MandelbrotAnimationRunner::*start)())
    {
        if (track.animation) {
            // Fractional stepping: at speed 1 we do exactly one step per frame,
            // matching the CCA runner. Higher speed means multiple steps per
            // frame; lower speed means we skip some frames.
            track.step_accumulator += speed;
            const int steps = static_cast<int>(track.step_accumulator);
            track.step_accumulator -= static_cast<float>(steps);
            for (int i = 0; i < steps; ++i) {
                track.animation->step();
                if (track.animation->finished()) break;
            }
            if (track.animation->finished()) {
                track.animation.reset();
                track.step_accumulator = 0.0f;
                track.cooldown_seconds = randomCooldown();
            }
            return;
        }

        // Cooldown ticks in real seconds scaled by speed so uninteresting
        // frames burn through cooldowns quickly.
        track.cooldown_seconds -= dt * speed;
        if (track.cooldown_seconds <= 0.0f) {
            track.cooldown_seconds = 0.0f;
            (this->*start)();
        }
    }

    void MandelbrotAnimationRunner::startOffset()
    {
        auto result = generator_.generateOffsetAnimation(offset_current_);
        offset_current_ = result.target;
        offset_track_.animation = std::move(result.animation);
    }

    void MandelbrotAnimationRunner::startStepSize()
    {
        auto result = generator_.generateStepSizeAnimation(step_size_current_);
        step_size_current_ = result.target;
        step_track_.animation = std::move(result.animation);
    }

    void MandelbrotAnimationRunner::startInitial()
    {
        auto result = generator_.generateInitialAnimation(initial_current_);
        initial_current_ = result.target;
        initial_track_.animation = std::move(result.animation);
    }

    void MandelbrotAnimationRunner::startPalette()
    {
        auto result = generator_.generatePaletteAnimation(palette_current_);
        palette_current_ = std::move(result.target);
        palette_track_.animation = std::move(result.animation);
    }
}
