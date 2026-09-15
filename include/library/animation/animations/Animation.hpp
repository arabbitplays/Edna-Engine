#ifndef EDNA_ENGINE_ANIMATION_HPP
#define EDNA_ENGINE_ANIMATION_HPP

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>

#include <library/animation/animations/IAnimation.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>

namespace Animation
{
    template <typename T>
    class Animation : public IAnimation
    {
    public:
        Animation(T start,
                  T target,
                  int step_count,
                  std::function<void(const T&)> on_update,
                  std::shared_ptr<EasingFunction> easing = std::make_shared<Linear>());

        void step() override;
        void reset() override;
        bool finished() const override;

    protected:
        virtual T interpolate(float t) const = 0;

        T start;
        T target;

    private:
        int step_count;
        int current_step;
        std::function<void(const T&)> on_update;
        std::shared_ptr<EasingFunction> easing_function;
    };

    template <typename T>
    Animation<T>::Animation(T start,
                            T target,
                            int step_count,
                            std::function<void(const T&)> on_update,
                            std::shared_ptr<EasingFunction> easing)
        : start(std::move(start)),
          target(std::move(target)),
          step_count(step_count > 0 ? step_count : 1),
          current_step(0),
          on_update(std::move(on_update)),
          easing_function(std::move(easing))
    {
    }

    template <typename T>
    void Animation<T>::step()
    {
        if (current_step >= step_count)
            return;

        ++current_step;

        const float raw = static_cast<float>(current_step) / static_cast<float>(step_count);
        const float t = easing_function->apply(std::clamp(raw, 0.0f, 1.0f));

        if (on_update)
            on_update(interpolate(t));
    }

    template <typename T>
    void Animation<T>::reset()
    {
        current_step = 0;
    }

    template <typename T>
    bool Animation<T>::finished() const
    {
        return current_step >= step_count;
    }
}

#endif //EDNA_ENGINE_ANIMATION_HPP
