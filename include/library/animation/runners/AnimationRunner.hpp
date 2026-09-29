#ifndef EDNA_ENGINE_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_ANIMATIONRUNNER_HPP

#include <chrono>
#include <library/animation/runners/AnimationTrack.hpp>
#include <memory>
#include <vector>

namespace Animation
{
    class AnimationRunner
    {
    public:
        AnimationRunner();

        void addTrack(std::unique_ptr<AnimationTrack> track);

        void update();

    private:
        std::vector<std::unique_ptr<AnimationTrack>> tracks_;
        std::chrono::steady_clock::time_point last_tick_;
    };
} // namespace Animation

#endif // EDNA_ENGINE_ANIMATIONRUNNER_HPP
