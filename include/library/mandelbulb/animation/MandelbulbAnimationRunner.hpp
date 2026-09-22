#ifndef EDNA_ENGINE_MANDELBULB_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_MANDELBULB_ANIMATIONRUNNER_HPP

#include <chrono>
#include <library/animation/animations/IAnimation.hpp>
#include <library/mandelbulb/animation/MandelbulbAnimationGenerator.hpp>

namespace mandelbulb
{
    // Drives three interpolated tracks (power, theta_offset, step_rotation_angle)
    // on independent random cooldowns, and periodically re-rolls the discrete
    // step_rotation_axis. Same shape as the CCA runner: after a track finishes,
    // its cooldown counts down; when it hits zero a new animation starts from
    // wherever the last one ended.
    class MandelbulbAnimationRunner
    {
    public:
        static constexpr float COOLDOWN_MIN_SECONDS = 1.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 4.0f;
        static constexpr float AXIS_COOLDOWN_MIN_SECONDS = 4.0f;
        static constexpr float AXIS_COOLDOWN_MAX_SECONDS = 10.0f;

        MandelbulbAnimationRunner(MandelbulbAnimationGenerator generator, float initial_power,
            float initial_theta_offset, float initial_step_rotation_angle);

        void update();

    private:
        struct Track
        {
            ::Animation::AnimationHandle animation;
            float cooldown_seconds = 0.0f;
        };

        void tick(Track& track, float dt, void (MandelbulbAnimationRunner::*start)());
        void startPower();
        void startThetaOffset();
        void startStepRotationAngle();
        void startStepRotationAxis();

        MandelbulbAnimationGenerator generator_;

        Track power_track_;
        float power_current_;

        Track theta_offset_track_;
        float theta_offset_current_;

        Track step_rotation_angle_track_;
        float step_rotation_angle_current_;

        Track step_rotation_axis_track_;
        glm::vec3 step_rotation_axis_current_;

        std::chrono::steady_clock::time_point last_tick_;
    };
} // namespace mandelbulb

#endif // EDNA_ENGINE_MANDELBULB_ANIMATIONRUNNER_HPP
