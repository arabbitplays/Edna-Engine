#ifndef EDNA_ENGINE_DISCRETEANIMATIONTRACK_HPP
#define EDNA_ENGINE_DISCRETEANIMATIONTRACK_HPP

#include <functional>
#include <library/animation/runners/AnimationTrack.hpp>

namespace Animation
{
    class DiscreteAnimationTrack : public AnimationTrack
    {
    public:
        DiscreteAnimationTrack(std::function<void()> generator, float cooldown_min_seconds,
            float cooldown_max_seconds);

        void start() override;
        void tick(float dt) override;

    private:
        float rollCooldown() const;

        std::function<void()> generator_;
        float cooldown_min_seconds_;
        float cooldown_max_seconds_;
        float cooldown_seconds_ = 0.0f;
    };
} // namespace Animation

#endif // EDNA_ENGINE_DISCRETEANIMATIONTRACK_HPP
