#ifndef EDNA_ENGINE_NEIGHBORHOOD_HPP
#define EDNA_ENGINE_NEIGHBORHOOD_HPP

#include <glm/vec2.hpp>
#include <vector>

namespace cellular_automaton
{
    struct Neighborhood
    {
        std::vector<glm::ivec2> offsets;
    };
} // namespace cellular_automaton

#endif // EDNA_ENGINE_NEIGHBORHOOD_HPP
