#ifndef EDNA_ENGINE_INTERPOLATEDANIMATIONTRACK_TPP
#define EDNA_ENGINE_INTERPOLATEDANIMATIONTRACK_TPP

#include <util/RandomUtil.hpp>

namespace Animation
{
    template <typename T>
    InterpolatedAnimationTrack<T>::InterpolatedAnimationTrack(
        T initial_value, Generator generator, float cooldown_min_seconds, float cooldown_max_seconds)
        : initial_value_(std::move(initial_value)), generator_(std::move(generator)),
          cooldown_min_seconds_(cooldown_min_seconds), cooldown_max_seconds_(cooldown_max_seconds)
    {
    }

    template <typename T> void InterpolatedAnimationTrack<T>::start()
    {
        if (generator_)
        {
            animation_ = generator_(initial_value_);
        }
    }

    template <typename T> void InterpolatedAnimationTrack<T>::tick(float dt)
    {
        if (animation_ && !animation_->finished())
        {
            animation_->step();
            if (animation_->finished())
            {
                cooldown_seconds_ = rollCooldown();
            }
            return;
        }

        cooldown_seconds_ -= dt;
        if (cooldown_seconds_ <= 0.0F)
        {
            cooldown_seconds_ = 0.0F;
            if (generator_)
            {
                animation_ = generator_(current());
            }
        }
    }

    template <typename T> float InterpolatedAnimationTrack<T>::rollCooldown() const
    {
        return cooldown_max_seconds_ > cooldown_min_seconds_
            ? RtEngine::RandomUtil::randomInRange(cooldown_min_seconds_, cooldown_max_seconds_)
            : cooldown_min_seconds_;
    }
} // namespace Animation

#endif // EDNA_ENGINE_INTERPOLATEDANIMATIONTRACK_TPP
