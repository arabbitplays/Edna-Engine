#include <library/animation/animations/AnimationBuilder.hpp>

#include <type_traits>
#include <utility>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <library/animation/animations/FloatAnimation.hpp>
#include <library/animation/animations/IntAnimation.hpp>
#include <library/animation/animations/VectorAnimation.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>

namespace Animation
{
    template <typename T>
    AnimationBuilder<T>& AnimationBuilder<T>::from(T value)
    {
        start_value = std::move(value);
        return *this;
    }

    template <typename T>
    AnimationBuilder<T>& AnimationBuilder<T>::to(T value)
    {
        target_value = std::move(value);
        return *this;
    }

    template <typename T>
    AnimationBuilder<T>& AnimationBuilder<T>::steps(int count)
    {
        step_count = count;
        return *this;
    }

    template <typename T>
    AnimationBuilder<T>& AnimationBuilder<T>::curve(EasingCurve curve)
    {
        curve_choice = curve;
        return *this;
    }

    template <typename T>
    AnimationBuilder<T>& AnimationBuilder<T>::direction(EasingDirection direction)
    {
        direction_choice = direction;
        return *this;
    }

    template <typename T>
    AnimationBuilder<T>& AnimationBuilder<T>::easing(std::shared_ptr<EasingFunction> easing)
    {
        easing_override = std::move(easing);
        return *this;
    }

    template <typename T>
    AnimationBuilder<T>& AnimationBuilder<T>::onUpdate(std::function<void(const T&)> on_update)
    {
        this->on_update = std::move(on_update);
        return *this;
    }

    template <typename T>
    std::unique_ptr<Animation<T>> AnimationBuilder<T>::build() const
    {
        auto easing = easing_override ? easing_override : makeEasingFunction(curve_choice, direction_choice);

        if constexpr (std::is_same_v<T, float>)
            return std::make_unique<FloatAnimation>(start_value, target_value, step_count, on_update, std::move(easing));
        else if constexpr (std::is_same_v<T, int>)
            return std::make_unique<IntAnimation>(start_value, target_value, step_count, on_update, std::move(easing));
        else
            return std::make_unique<VectorAnimation<T>>(start_value, target_value, step_count, on_update, std::move(easing));
    }

    template class AnimationBuilder<float>;
    template class AnimationBuilder<int>;
    template class AnimationBuilder<glm::vec2>;
    template class AnimationBuilder<glm::vec3>;
    template class AnimationBuilder<glm::vec4>;
}
