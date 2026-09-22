#ifndef EDNA_ENGINE_FLOATANIMATION_HPP
#define EDNA_ENGINE_FLOATANIMATION_HPP

#include <library/animation/animations/Animation.hpp>

namespace Animation
{
    class FloatAnimation : public Animation<float>
    {
    public:
        using Animation<float>::Animation;

    protected:
        float interpolate(float t) const override;
    };
} // namespace Animation

#endif // EDNA_ENGINE_FLOATANIMATION_HPP
