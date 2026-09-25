#include <library/animation/runners/AnimationRunner.hpp>
#include <utility>

namespace Animation
{
    AnimationRunner::AnimationRunner() : last_tick_(std::chrono::steady_clock::now())
    {
    }

    void AnimationRunner::addTrack(std::unique_ptr<AnimationTrack> track)
    {
        if (!track)
        {
            return;
        }
        track->start();
        tracks_.push_back(std::move(track));
    }

    void AnimationRunner::update()
    {
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last_tick_).count();
        last_tick_ = now;

        for (auto& track : tracks_)
        {
            track->tick(dt);
        }
    }
} // namespace Animation
