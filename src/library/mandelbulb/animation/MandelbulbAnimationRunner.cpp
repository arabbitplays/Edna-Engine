#include <cstdint>
#include <library/mandelbulb/animation/MandelbulbAnimationRunner.hpp>
#include <limits>
#include <util/RandomUtil.hpp>
#include <utility>

namespace mandelbulb
{
    namespace
    {
        float randomCooldown(float min_seconds, float max_seconds)
        {
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return min_seconds + (t * (max_seconds - min_seconds));
        }

        float randomTrackCooldown()
        {
            using Runner = MandelbulbAnimationRunner;
            return randomCooldown(Runner::COOLDOWN_MIN_SECONDS, Runner::COOLDOWN_MAX_SECONDS);
        }

        float randomAxisCooldown()
        {
            using Runner = MandelbulbAnimationRunner;
            return randomCooldown(Runner::AXIS_COOLDOWN_MIN_SECONDS, Runner::AXIS_COOLDOWN_MAX_SECONDS);
        }
    } // namespace

    MandelbulbAnimationRunner::MandelbulbAnimationRunner(MandelbulbAnimationGenerator generator, float initial_power,
        float initial_theta_offset, float initial_step_rotation_angle)
        : generator_(std::move(generator)), power_current_(initial_power),
          theta_offset_current_(initial_theta_offset),
          step_rotation_angle_current_(initial_step_rotation_angle),
          last_tick_(std::chrono::steady_clock::now())
    {
        startPower();
        startThetaOffset();
        startStepRotationAngle();
        startStepRotationAxis();
    }

    void MandelbulbAnimationRunner::update()
    {
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last_tick_).count();
        last_tick_ = now;

        tick(power_track_, dt, &MandelbulbAnimationRunner::startPower);
        tick(theta_offset_track_, dt, &MandelbulbAnimationRunner::startThetaOffset);
        tick(step_rotation_angle_track_, dt, &MandelbulbAnimationRunner::startStepRotationAngle);
        tick(step_rotation_axis_track_, dt, &MandelbulbAnimationRunner::startStepRotationAxis);
    }

    void MandelbulbAnimationRunner::tick(
        Track& track, float dt, void (MandelbulbAnimationRunner::*start)())
    {
        if (track.animation)
        {
            track.animation->step();
            if (track.animation->finished())
            {
                track.animation.reset();
                track.cooldown_seconds = randomTrackCooldown();
            }
            return;
        }

        track.cooldown_seconds -= dt;
        if (track.cooldown_seconds <= 0.0F)
        {
            track.cooldown_seconds = 0.0F;
            (this->*start)();
        }
    }

    void MandelbulbAnimationRunner::startPower()
    {
        auto result = generator_.generatePowerAnimation(power_current_);
        power_current_ = result.target;
        power_track_.animation = std::move(result.animation);
    }

    void MandelbulbAnimationRunner::startThetaOffset()
    {
        auto result = generator_.generateThetaOffsetAnimation(theta_offset_current_);
        theta_offset_current_ = result.target;
        theta_offset_track_.animation = std::move(result.animation);
    }

    void MandelbulbAnimationRunner::startStepRotationAngle()
    {
        auto result = generator_.generateStepRotationAngleAnimation(step_rotation_angle_current_);
        step_rotation_angle_current_ = result.target;
        step_rotation_angle_track_.animation = std::move(result.animation);
    }

    void MandelbulbAnimationRunner::startStepRotationAxis()
    {
        auto result = generator_.generateStepRotationAxisAnimation(step_rotation_axis_current_);
        step_rotation_axis_current_ = result.target;
        step_rotation_axis_track_.animation = std::move(result.animation);
    }
} // namespace mandelbulb
