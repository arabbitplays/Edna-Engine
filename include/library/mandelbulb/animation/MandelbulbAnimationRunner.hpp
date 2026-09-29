#ifndef EDNA_ENGINE_MANDELBULB_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_MANDELBULB_ANIMATIONRUNNER_HPP

#include <functional>
#include <glm/vec3.hpp>
#include <library/animation/runners/AnimationRunner.hpp>
#include <library/mandelbulb/animation/MandelbulbAnimationGenerator.hpp>

namespace mandelbulb
{
    class MandelbulbAnimationRunner
    {
    public:
        static constexpr float COOLDOWN_MIN_SECONDS = 1.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 4.0f;

        MandelbulbAnimationRunner(std::function<void(float)> set_power,
            std::function<void(float)> set_theta_offset, std::function<void(float)> set_step_rotation_angle,
            std::function<void(const glm::vec3&)> set_step_rotation_axis, float initial_power,
            float initial_theta_offset, float initial_step_rotation_angle,
            const glm::vec3& initial_step_rotation_axis);

        void update()
        {
            runner_.update();
        }

    private:
        MandelbulbAnimationGenerator generator_;
        ::Animation::AnimationRunner runner_;
    };
} // namespace mandelbulb

#endif // EDNA_ENGINE_MANDELBULB_ANIMATIONRUNNER_HPP
