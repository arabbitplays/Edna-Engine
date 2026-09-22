#include <library/animation/curves/AnimationCurve.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>

#include <utility>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>


namespace Animation
{
    template <typename T, typename AnimT>
    AnimationCurve<T, AnimT>& AnimationCurve<T, AnimT>::add(FixPoint point)
    {
        points.push_back(std::move(point));
        return *this;
    }

    template <typename T, typename AnimT>
    AnimationCurve<T, AnimT>& AnimationCurve<T, AnimT>::onUpdate(std::function<void(const T&)> on_update)
    {
        this->on_update = std::move(on_update);
        return *this;
    }

    template <typename T, typename AnimT>
    std::vector<std::unique_ptr<Animation<T>>> AnimationCurve<T, AnimT>::build() const
    {
        std::vector<std::unique_ptr<Animation<T>>> segments;
        if (points.size() < 2) {
            return segments;
}

        segments.reserve(points.size() - 1);
        for (size_t i = 1; i < points.size(); ++i)
        {
            const auto& prev = points[i - 1];
            const auto& cur = points[i];

            auto easing = cur.easing_override
                              ? cur.easing_override
                              : makeEasingFunction(cur.curve, cur.direction);

            segments.emplace_back(std::make_unique<AnimT>(
                prev.value, cur.value, cur.steps, on_update, std::move(easing)));
        }
        return segments;
    }

    template class AnimationCurve<float, FloatAnimation>;
    template class AnimationCurve<int, IntAnimation>;
    template class AnimationCurve<glm::vec2, VectorAnimation<glm::vec2>>;
    template class AnimationCurve<glm::vec3, VectorAnimation<glm::vec3>>;
    template class AnimationCurve<glm::vec4, VectorAnimation<glm::vec4>>;
}
