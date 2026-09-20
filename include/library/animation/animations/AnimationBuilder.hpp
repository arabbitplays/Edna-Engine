#ifndef EDNA_ENGINE_ANIMATIONBUILDER_HPP
#define EDNA_ENGINE_ANIMATIONBUILDER_HPP

#include <functional>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <library/animation/animations/Animation.hpp>
#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>

namespace Animation
{
    template <typename T>
    class AnimationBuilder
    {
    public:
        AnimationBuilder& from(T value);
        AnimationBuilder& to(T value);
        AnimationBuilder& steps(int count);
        AnimationBuilder& curve(EasingCurve curve);
        AnimationBuilder& direction(EasingDirection direction);
        AnimationBuilder& easing(std::shared_ptr<EasingFunction> easing);
        AnimationBuilder& onUpdate(std::function<void(const T&)> on_update);

        std::unique_ptr<Animation<T>> build() const;

    private:
        T start_value{};
        T target_value{};
        int step_count = 1;
        EasingCurve curve_choice = EasingCurve::Linear;
        EasingDirection direction_choice = EasingDirection::InOut;
        std::shared_ptr<EasingFunction> easing_override;
        std::function<void(const T&)> on_update;
    };

    using FloatAnimationBuilder = AnimationBuilder<float>;
    using IntAnimationBuilder = AnimationBuilder<int>;

    using Vec2AnimationBuilder = AnimationBuilder<glm::vec2>;
    using Vec3AnimationBuilder = AnimationBuilder<glm::vec3>;
    using Vec4AnimationBuilder = AnimationBuilder<glm::vec4>;
}

#endif //EDNA_ENGINE_ANIMATIONBUILDER_HPP
