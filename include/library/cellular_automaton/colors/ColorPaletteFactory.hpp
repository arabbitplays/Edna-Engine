#ifndef EDNA_ENGINE_COLORPALETTEFACTORY_HPP
#define EDNA_ENGINE_COLORPALETTEFACTORY_HPP

#include <library/cellular_automaton/colors/ColorPalette.hpp>
#include <library/cellular_automaton/colors/ColorPaletteName.hpp>

namespace cellular_automaton
{
    class ColorPaletteFactory
    {
    public:
        static ColorPalette create(ColorPaletteName name);
    };
}

#endif //EDNA_ENGINE_COLORPALETTEFACTORY_HPP
