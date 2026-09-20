#ifndef EDNA_ENGINE_COLORPALETTE_HPP
#define EDNA_ENGINE_COLORPALETTE_HPP

#include <vector>

#include <glm/vec4.hpp>

namespace color
{
    struct ColorPalette
    {
        std::vector<glm::vec4> colors;
    };
}

#endif //EDNA_ENGINE_COLORPALETTE_HPP
