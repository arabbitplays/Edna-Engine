#include <library/animation/animations/FloatAnimation.hpp>

namespace Animation
{
    float FloatAnimation::interpolate(float t) const
    {
        return start + ((target - start) * t);
    }

    template class Animation<float>;
} // namespace Animation
