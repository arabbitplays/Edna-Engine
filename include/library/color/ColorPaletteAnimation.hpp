#ifndef EDNA_ENGINE_COLORPALETTEANIMATION_HPP
#define EDNA_ENGINE_COLORPALETTEANIMATION_HPP

#include <library/animation/animations/Animation.hpp>
#include <library/color/ColorPalette.hpp>

namespace color
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
