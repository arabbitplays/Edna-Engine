#ifndef EDNA_ENGINE_COLORPALETTE_HPP
#define EDNA_ENGINE_COLORPALETTE_HPP

#include <vector>

#include <glm/vec3.hpp>

namespace cellular_automaton
{
    struct ColorPalette
    {
        std::vector<glm::vec3> colors;
    };
}

#endif //EDNA_ENGINE_COLORPALETTE_HPP
