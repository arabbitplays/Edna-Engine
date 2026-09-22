#ifndef EDNA_ENGINE_COLORPALETTE_HPP
#define EDNA_ENGINE_COLORPALETTE_HPP

#include <glm/vec4.hpp>
#include <vector>

namespace color
{
    struct ColorPalette
    {
        std::vector<glm::vec4> colors;
    };
} // namespace color

#endif // EDNA_ENGINE_COLORPALETTE_HPP
