#ifndef EDNA_ENGINE_NEIGHBORHOODFACTORY_HPP
#define EDNA_ENGINE_NEIGHBORHOODFACTORY_HPP

#include <library/cellular_automaton/neighborhoods/Neighborhood.hpp>
#include <library/cellular_automaton/neighborhoods/NeighborhoodShape.hpp>

namespace cellular_automaton
{
    class NeighborhoodFactory
    {
    public:
        static Neighborhood create(NeighborhoodShape shape, int size);
    };
}

#endif //EDNA_ENGINE_NEIGHBORHOODFACTORY_HPP
