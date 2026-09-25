#ifndef EDNA_ENGINE_ANIMATIONTRACK_HPP
#define EDNA_ENGINE_ANIMATIONTRACK_HPP

namespace Animation
{
    class AnimationTrack
    {
    public:
        virtual ~AnimationTrack() = default;

        virtual void start() = 0;
        virtual void tick(float dt) = 0;
    };
} // namespace Animation

#endif // EDNA_ENGINE_ANIMATIONTRACK_HPP
