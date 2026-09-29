#ifndef EDNA_ENGINE_INTERPOLATEDANIMATIONTRACK_HPP
#define EDNA_ENGINE_INTERPOLATEDANIMATIONTRACK_HPP

#include <algorithm>
#include <functional>
#include <library/animation/animations/Animation.hpp>
#include <library/animation/runners/AnimationTrack.hpp>
#include <memory>
#include <utility>

namespace Animation
{
    // Runs an Animation<T> to completion, then waits a random cooldown, then
    // asks `generator` for the next animation using the currently-visible
    // value as its starting point. Because the track reads `current()` off the
    // animation itself, callers no longer keep a shadow copy of the value.
    template <typename T> class InterpolatedAnimationTrack : public AnimationTrack
    {
    public:
        using AnimationPtr = std::shared_ptr<Animation<T>>;
        using Generator = std::function<AnimationPtr(const T& current)>;

        InterpolatedAnimationTrack(
            T initial_value, Generator generator, float cooldown_min_seconds, float cooldown_max_seconds);

        void start() override;
        void tick(float dt) override;

        // Last value emitted by the active or most-recently-finished animation
        // (falls back to the initial value before start()).
        const T& current() const
        {
            return animation_ ? animation_->current() : initial_value_;
        }

    private:
        float rollCooldown() const;

        T initial_value_;
        Generator generator_;
        float cooldown_min_seconds_;
        float cooldown_max_seconds_;
        AnimationPtr animation_;
        float cooldown_seconds_ = 0.0f;
    };
} // namespace Animation

#include <library/animation/runners/InterpolatedAnimationTrack.tpp>

#endif // EDNA_ENGINE_INTERPOLATEDANIMATIONTRACK_HPP
