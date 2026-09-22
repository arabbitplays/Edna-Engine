#ifndef EDNA_ENGINE_COLORPALETTEFACTORY_HPP
#define EDNA_ENGINE_COLORPALETTEFACTORY_HPP

#include <library/color/ColorPalette.hpp>
#include <library/color/ColorPaletteName.hpp>

namespace color
{
    class ColorPaletteFactory
    {
    public:
        static ColorPalette create(ColorPaletteName name);
    };
} // namespace color

#endif // EDNA_ENGINE_COLORPALETTEFACTORY_HPP
