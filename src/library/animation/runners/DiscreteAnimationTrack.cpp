#include <library/animation/runners/DiscreteAnimationTrack.hpp>
#include <util/RandomUtil.hpp>
#include <utility>

namespace Animation
{
    DiscreteAnimationTrack::DiscreteAnimationTrack(
        std::function<void()> generator, float cooldown_min_seconds, float cooldown_max_seconds)
        : generator_(std::move(generator)), cooldown_min_seconds_(cooldown_min_seconds),
          cooldown_max_seconds_(cooldown_max_seconds)
    {
    }

    void DiscreteAnimationTrack::start()
    {
        if (generator_)
        {
            generator_();
        }
        cooldown_seconds_ = rollCooldown();
    }

    void DiscreteAnimationTrack::tick(float dt)
    {
        cooldown_seconds_ -= dt;
        if (cooldown_seconds_ <= 0.0F)
        {
            if (generator_)
            {
                generator_();
            }
            cooldown_seconds_ = rollCooldown();
        }
    }

    float DiscreteAnimationTrack::rollCooldown() const
    {
        return cooldown_max_seconds_ > cooldown_min_seconds_
            ? RtEngine::RandomUtil::randomInRange(cooldown_min_seconds_, cooldown_max_seconds_)
            : cooldown_min_seconds_;
    }
} // namespace Animation
