#include <library/animation/animations/IntAnimation.hpp>

#include <cmath>

namespace Animation
{
    int IntAnimation::interpolate(float t) const
    {
        const float a = static_cast<float>(start);
        const float b = static_cast<float>(target);
        return static_cast<int>(std::lround(a + (b - a) * t));
    }

    template class Animation<int>;
}
