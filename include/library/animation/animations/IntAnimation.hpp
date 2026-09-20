#ifndef EDNA_ENGINE_INTANIMATION_HPP
#define EDNA_ENGINE_INTANIMATION_HPP

#include <library/animation/animations/Animation.hpp>

namespace Animation
{
    class IntAnimation : public Animation<int>
    {
    public:
        using Animation<int>::Animation;

    protected:
        int interpolate(float t) const override;
    };
}

#endif //EDNA_ENGINE_INTANIMATION_HPP
