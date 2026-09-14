#ifndef EDNA_ENGINE_NEIGHBORHOOD_HPP
#define EDNA_ENGINE_NEIGHBORHOOD_HPP

#include <vector>

#include <glm/vec2.hpp>

namespace cellular_automaton
{
    struct Neighborhood
    {
        std::vector<glm::ivec2> offsets;
    };
}

#endif //EDNA_ENGINE_NEIGHBORHOOD_HPP
