#include <library/animation/runners/InterpolatedAnimationTrack.hpp>
#include <library/mandelbulb/animation/MandelbulbAnimationRunner.hpp>
#include <memory>
#include <utility>

namespace mandelbulb
{
    MandelbulbAnimationRunner::MandelbulbAnimationRunner(std::function<void(float)> set_power,
        std::function<void(float)> set_theta_offset, std::function<void(float)> set_step_rotation_angle,
        std::function<void(const glm::vec3&)> set_step_rotation_axis, float initial_power,
        float initial_theta_offset, float initial_step_rotation_angle,
        const glm::vec3& initial_step_rotation_axis)
        : generator_(std::move(set_power), std::move(set_theta_offset), std::move(set_step_rotation_angle),
              std::move(set_step_rotation_axis))
    {
        runner_.addTrack(std::make_unique<::Animation::InterpolatedAnimationTrack<float>>(
            initial_power,
            [this](const float& current) -> std::shared_ptr<::Animation::Animation<float>>
            { return std::move(generator_.generatePowerAnimation(current).animation); },
            COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS));

        runner_.addTrack(std::make_unique<::Animation::InterpolatedAnimationTrack<float>>(
            initial_theta_offset,
            [this](const float& current) -> std::shared_ptr<::Animation::Animation<float>>
            { return std::move(generator_.generateThetaOffsetAnimation(current).animation); },
            COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS));

        runner_.addTrack(std::make_unique<::Animation::InterpolatedAnimationTrack<float>>(
            initial_step_rotation_angle,
            [this](const float& current) -> std::shared_ptr<::Animation::Animation<float>>
            { return std::move(generator_.generateStepRotationAngleAnimation(current).animation); },
            COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS));

        runner_.addTrack(std::make_unique<::Animation::InterpolatedAnimationTrack<glm::vec3>>(
            initial_step_rotation_axis,
            [this](const glm::vec3& current) -> std::shared_ptr<::Animation::Animation<glm::vec3>>
            { return std::move(generator_.generateStepRotationAxisAnimation(current).animation); },
            COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS));
    }
} // namespace mandelbulb
