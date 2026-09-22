#ifndef EDNA_ENGINE_MANDELBULB_ANIMATIONGENERATOR_HPP
#define EDNA_ENGINE_MANDELBULB_ANIMATIONGENERATOR_HPP

#include <functional>
#include <glm/vec3.hpp>
#include <library/animation/animations/FloatAnimation.hpp>
#include <memory>

namespace mandelbulb
{
    class MandelbulbAnimationGenerator
    {
    public:
        static constexpr int STEP_COUNT_MIN = 400;
        static constexpr int STEP_COUNT_MAX = 2400;

        static constexpr float POWER_MIN = 4.0f;
        static constexpr float POWER_MAX = 12.0f;

        static constexpr float THETA_OFFSET_MIN = -3.14159265f;
        static constexpr float THETA_OFFSET_MAX = 3.14159265f;

        static constexpr float STEP_ROTATION_ANGLE_MIN = -0.5f;
        static constexpr float STEP_ROTATION_ANGLE_MAX = 0.5f;

        MandelbulbAnimationGenerator(std::function<void(float)> set_power,
            std::function<void(float)> set_theta_offset, std::function<void(float)> set_step_rotation_angle,
            std::function<void(const glm::vec3&)> set_step_rotation_axis);

        struct FloatAnimationResult
        {
            std::unique_ptr<::Animation::FloatAnimation> animation;
            float target;
        };

        FloatAnimationResult generatePowerAnimation(float current);
        FloatAnimationResult generateThetaOffsetAnimation(float current);
        FloatAnimationResult generateStepRotationAngleAnimation(float current);

        // Non-interpolated re-roll: the axis snaps to a fresh unit vector so
        // the compound step rotation swings through a new orientation.
        void applyRandomStepRotationAxis();

    private:
        std::function<void(float)> set_power_;
        std::function<void(float)> set_theta_offset_;
        std::function<void(float)> set_step_rotation_angle_;
        std::function<void(const glm::vec3&)> set_step_rotation_axis_;
    };
} // namespace mandelbulb

#endif // EDNA_ENGINE_MANDELBULB_ANIMATIONGENERATOR_HPP
