#ifndef EDNA_ENGINE_BEZIERANIMATION_HPP
#define EDNA_ENGINE_BEZIERANIMATION_HPP

#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <library/animation/animations/Animation.hpp>
#include <memory>
#include <utility>

namespace Animation
{
    template <typename T> class BezierAnimation : public Animation<T>
    {
    public:
        BezierAnimation(T p0, T p1, T p2, T p3, int step_count, std::function<void(const T&)> on_update,
            std::shared_ptr<EasingFunction> easing = std::make_shared<Linear>())
            : Animation<T>(std::move(p0), std::move(p3), step_count, std::move(on_update), std::move(easing)),
              c1_(std::move(p1)), c2_(std::move(p2))
        {
        }

    protected:
        T interpolate(float t) const override;

    private:
        T c1_;
        T c2_;
    };

    using Vec2BezierAnimation = BezierAnimation<glm::vec2>;
    using Vec3BezierAnimation = BezierAnimation<glm::vec3>;
    using Vec4BezierAnimation = BezierAnimation<glm::vec4>;
} // namespace Animation

#endif // EDNA_ENGINE_BEZIERANIMATION_HPP
