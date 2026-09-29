#ifndef EDNA_ENGINE_IANIMATION_HPP
#define EDNA_ENGINE_IANIMATION_HPP

#include <memory>

namespace Animation
{
    // Non-template base for polymorphic storage of animations with different value types.
    class IAnimation
    {
    public:
        virtual ~IAnimation() = default;

        virtual void step() = 0;
        virtual bool finished() const = 0;
    };

    using AnimationHandle = std::shared_ptr<IAnimation>;
} // namespace Animation

#endif // EDNA_ENGINE_IANIMATION_HPP
