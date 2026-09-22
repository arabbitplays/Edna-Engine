#include <array>
#include <cmath>
#include <cstdint>
#include <format>
#include <glm/geometric.hpp>
#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>
#include <library/mandelbulb/animation/MandelbulbAnimationGenerator.hpp>
#include <limits>
#include <logging/LogManager.hpp>
#include <memory>
#include <util/RandomUtil.hpp>
#include <utility>

namespace mandelbulb
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance =
                Logging::LogManager::getClassLogger<MandelbulbAnimationGenerator>();
            return instance;
        }

        float randomFloat(float min, float max)
        {
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return min + (t * (max - min));
        }

        int randomStepCount()
        {
            using Gen = MandelbulbAnimationGenerator;
            const auto range = static_cast<uint32_t>(Gen::STEP_COUNT_MAX - Gen::STEP_COUNT_MIN + 1);
            return Gen::STEP_COUNT_MIN + static_cast<int>(RtEngine::RandomUtil::generateInt() % range);
        }

        std::shared_ptr<::Animation::EasingFunction> randomInOutEasing()
        {
            constexpr std::array curves = {
                ::Animation::EasingCurve::Linear,
                ::Animation::EasingCurve::Cubic,
                ::Animation::EasingCurve::Elastic,
                ::Animation::EasingCurve::Bounce,
            };
            const std::size_t idx = RtEngine::RandomUtil::generateInt() % curves.size();
            return ::Animation::makeEasingFunction(curves[idx], ::Animation::EasingDirection::InOut);
        }

        std::unique_ptr<::Animation::FloatAnimation> makeFloatAnimation(
            float current, float target, std::function<void(float)> set)
        {
            return std::make_unique<::Animation::FloatAnimation>(
                current, target, randomStepCount(),
                [set](const float& v)
                {
                    if (set)
                    {
                        set(v);
                    }
                },
                randomInOutEasing());
        }
    } // namespace

    MandelbulbAnimationGenerator::MandelbulbAnimationGenerator(std::function<void(float)> set_power,
        std::function<void(float)> set_theta_offset, std::function<void(float)> set_step_rotation_angle,
        std::function<void(const glm::vec3&)> set_step_rotation_axis)
        : set_power_(std::move(set_power)), set_theta_offset_(std::move(set_theta_offset)),
          set_step_rotation_angle_(std::move(set_step_rotation_angle)),
          set_step_rotation_axis_(std::move(set_step_rotation_axis))
    {
    }

    MandelbulbAnimationGenerator::FloatAnimationResult MandelbulbAnimationGenerator::generatePowerAnimation(
        float current)
    {
        const float target = randomFloat(POWER_MIN, POWER_MAX);
        logger()->info(std::format("rolled power target = {:.3f}", target));
        return {.animation = makeFloatAnimation(current, target, set_power_), .target = target};
    }

    MandelbulbAnimationGenerator::FloatAnimationResult MandelbulbAnimationGenerator::generateThetaOffsetAnimation(
        float current)
    {
        const float target = randomFloat(THETA_OFFSET_MIN, THETA_OFFSET_MAX);
        logger()->info(std::format("rolled theta_offset target = {:.3f}", target));
        return {.animation = makeFloatAnimation(current, target, set_theta_offset_), .target = target};
    }

    MandelbulbAnimationGenerator::FloatAnimationResult
    MandelbulbAnimationGenerator::generateStepRotationAngleAnimation(float current)
    {
        const float target = randomFloat(STEP_ROTATION_ANGLE_MIN, STEP_ROTATION_ANGLE_MAX);
        logger()->info(std::format("rolled step_rotation_angle target = {:.3f}", target));
        return {.animation = makeFloatAnimation(current, target, set_step_rotation_angle_), .target = target};
    }

    void MandelbulbAnimationGenerator::applyRandomStepRotationAxis()
    {
        // Uniform-ish point on the unit sphere, biased slightly away from
        // degenerate zero vectors by the length check.
        glm::vec3 axis{
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f),
        };
        if (glm::length(axis) < 1e-3f)
        {
            axis = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        axis = glm::normalize(axis);
        logger()->info(std::format("rolled step_rotation_axis = ({:.2f},{:.2f},{:.2f})", axis.x, axis.y, axis.z));

        if (set_step_rotation_axis_)
        {
            set_step_rotation_axis_(axis);
        }
    }
} // namespace mandelbulb
