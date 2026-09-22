#ifndef EDNA_ENGINE_ANIMATIONCURVE_HPP
#define EDNA_ENGINE_ANIMATIONCURVE_HPP

#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <library/animation/animations/Animation.hpp>
#include <library/animation/animations/FloatAnimation.hpp>
#include <library/animation/animations/IntAnimation.hpp>
#include <library/animation/animations/VectorAnimation.hpp>
#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <memory>
#include <type_traits>
#include <vector>

namespace Animation
{
    template <typename T, typename AnimT> class AnimationCurve
    {
    public:
        static_assert(std::is_base_of_v<Animation<T>, AnimT>, "AnimT must derive from Animation<T>");

        struct FixPoint
        {
            T value{};
            int steps = 1;
            EasingCurve curve = EasingCurve::Linear;
            EasingDirection direction = EasingDirection::InOut;
            std::shared_ptr<EasingFunction> easing_override;
        };

        AnimationCurve& add(FixPoint point);
        AnimationCurve& onUpdate(std::function<void(const T&)> on_update);

        std::vector<std::unique_ptr<Animation<T>>> build() const;

    private:
        std::vector<FixPoint> points;
        std::function<void(const T&)> on_update;
    };

    using FloatAnimationCurve = AnimationCurve<float, FloatAnimation>;
    using IntAnimationCurve = AnimationCurve<int, IntAnimation>;

    using Vec2AnimationCurve = AnimationCurve<glm::vec2, VectorAnimation<glm::vec2>>;
    using Vec3AnimationCurve = AnimationCurve<glm::vec3, VectorAnimation<glm::vec3>>;
    using Vec4AnimationCurve = AnimationCurve<glm::vec4, VectorAnimation<glm::vec4>>;
} // namespace Animation

#endif // EDNA_ENGINE_ANIMATIONCURVE_HPP
