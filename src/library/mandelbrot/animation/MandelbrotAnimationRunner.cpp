#include <library/mandelbrot/animation/MandelbrotAnimationRunner.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

#include <library/mandelbrot/animation/MandelbrotProbe.hpp>
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
        std::function<void(const glm::vec2&)>             set_offset,
        std::function<void(float)>                        set_step_size,
        std::function<void(const glm::vec2&)>             set_initial,
        std::function<void(const ::color::ColorPalette&)> set_palette,
        MandelbrotState       initial_state,
        ::color::ColorPalette initial_palette)
        : current_state_(initial_state),
          palette_current_(std::move(initial_palette)),
          // Tee each setter through a wrapper that also updates current_state_
          // so a mid-animation restart chains from the on-screen value rather
          // than the previous animation's unreached target.
          generator_(
              [this, s = std::move(set_offset)](const glm::vec2& v) {
                  current_state_.offset = v;
                  if (s) s(v);
              },
              [this, s = std::move(set_step_size)](float v) {
                  current_state_.step_size = v;
                  if (s) s(v);
              },
              [this, s = std::move(set_initial)](const glm::vec2& v) {
                  current_state_.initial = v;
                  if (s) s(v);
              },
              [this, s = std::move(set_palette)](const ::color::ColorPalette& p) {
                  palette_current_ = p;
                  if (s) s(p);
              }),
          last_tick_(std::chrono::steady_clock::now())
    {
        startOffset();
        startStepSize();
        startInitial();
        startPalette();
    }

    void MandelbrotAnimationRunner::update(const glm::vec2& view_center, float view_span, bool julia_mode)
    {
        current_state_.julia_mode = julia_mode;
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last_tick_).count();
        last_tick_ = now;

        // Probe the actual on-screen view so speed reflects what the user
        // is looking at, not a canonical reference. Cache the score for
        // the step_size track's zoom coupling.
        float target_speed = 1.0f;
        if (view_span > 0.0f) {
            const ProbeResult probe = probeInterest(
                static_cast<double>(view_center.x),
                static_cast<double>(view_center.y),
                static_cast<double>(view_span),
                static_cast<double>(current_state_.initial.x),
                static_cast<double>(current_state_.initial.y),
                julia_mode,
                MandelbrotAnimationGenerator::PROBE_MAX_ITER,
                MandelbrotAnimationGenerator::PROBE_GRID_SIZE);
            last_view_edge_score_ = probe.edge_score;
            target_speed          = computeSpeed(probe.edge_score);
        } else {
            last_view_edge_score_ = 0.0f;
        }

        const float alpha = 1.0f - std::exp(-dt / SPEED_SMOOTHING_TAU_SECONDS);
        smoothed_speed_ += (target_speed - smoothed_speed_) * alpha;

        tick(offset_track_,  dt, smoothed_speed_, &MandelbrotAnimationRunner::startOffset);
        tick(step_track_,    dt, smoothed_speed_, &MandelbrotAnimationRunner::startStepSize);
        tick(initial_track_, dt, smoothed_speed_, &MandelbrotAnimationRunner::startInitial);
        tick(palette_track_, dt, smoothed_speed_, &MandelbrotAnimationRunner::startPalette);
    }

    float MandelbrotAnimationRunner::computeSpeed(float edge_score) const
    {
        const float x = std::clamp(edge_score / EDGE_SCORE_SATURATION, 0.0f, 1.0f);
        return MAX_SPEED - (MAX_SPEED - MIN_SPEED) * x;
    }

    void MandelbrotAnimationRunner::tick(
        Track& track, float dt, float speed,
        void (MandelbrotAnimationRunner::*start)())
    {
        if (track.animation) {
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

        track.cooldown_seconds -= dt * speed;
        if (track.cooldown_seconds <= 0.0f) {
            track.cooldown_seconds = 0.0f;
            (this->*start)();
        }
    }

    void MandelbrotAnimationRunner::startOffset()
    {
        auto result = generator_.generateOffsetAnimation(current_state_);
        offset_track_.animation = std::move(result.animation);
    }

    void MandelbrotAnimationRunner::startStepSize()
    {
        auto result = generator_.generateStepSizeAnimation(current_state_, last_view_edge_score_);
        step_track_.animation = std::move(result.animation);
    }

    void MandelbrotAnimationRunner::startInitial()
    {
        auto result = generator_.generateInitialAnimation(current_state_);
        initial_track_.animation = std::move(result.animation);
    }

    void MandelbrotAnimationRunner::startPalette()
    {
        auto result = generator_.generatePaletteAnimation(palette_current_);
        palette_track_.animation = std::move(result.animation);
    }
}
