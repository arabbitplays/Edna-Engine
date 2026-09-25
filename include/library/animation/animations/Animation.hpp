#ifndef EDNA_ENGINE_ANIMATION_HPP
#define EDNA_ENGINE_ANIMATION_HPP

#include <algorithm>
#include <functional>
#include <library/animation/animations/IAnimation.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <memory>
#include <utility>

namespace Animation
{
    template <typename T> class Animation : public IAnimation
    {
    public:
        Animation(T start, T target, int step_count, std::function<void(const T&)> on_update,
            std::shared_ptr<EasingFunction> easing = std::make_shared<Linear>());

        void step() override;
        bool finished() const override;

        const T& current() const
        {
            return current_value;
        }

    protected:
        virtual T interpolate(float t) const = 0;

        T start;
        T target;

    private:
        int step_count;
        int current_step;
        T current_value;
        std::function<void(const T&)> on_update;
        std::shared_ptr<EasingFunction> easing_function;
    };

    template <typename T>
    Animation<T>::Animation(T start, T target, int step_count, std::function<void(const T&)> on_update,
        std::shared_ptr<EasingFunction> easing)
        : start(std::move(start)), target(std::move(target)), step_count(step_count > 0 ? step_count : 1),
          current_step(0), current_value(this->start), on_update(std::move(on_update)),
          easing_function(std::move(easing))
    {
    }

    template <typename T> void Animation<T>::step()
    {
        if (current_step >= step_count)
            return;

        ++current_step;

        const float raw = static_cast<float>(current_step) / static_cast<float>(step_count);
        const float t = easing_function->apply(std::clamp(raw, 0.0f, 1.0f));

        current_value = interpolate(t);
        if (on_update)
            on_update(current_value);
    }

    template <typename T> bool Animation<T>::finished() const
    {
        return current_step >= step_count;
    }
} // namespace Animation

#endif // EDNA_ENGINE_ANIMATION_HPP
