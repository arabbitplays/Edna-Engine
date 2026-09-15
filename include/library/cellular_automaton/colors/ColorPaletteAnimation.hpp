#ifndef EDNA_ENGINE_COLORPALETTEANIMATION_HPP
#define EDNA_ENGINE_COLORPALETTEANIMATION_HPP

#include <library/cellular_automaton/colors/ColorPalette.hpp>
#include <library/animation/animations/Animation.hpp>

namespace cellular_automaton
{
    class ColorPaletteAnimation : public ::Animation::Animation<ColorPalette>
    {
    public:
        using Base = ::Animation::Animation<ColorPalette>;
        using Base::Base;

    protected:
        ColorPalette interpolate(float t) const override;
    };
}

#endif //EDNA_ENGINE_COLORPALETTEANIMATION_HPP
