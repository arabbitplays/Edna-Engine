#include <library/cellular_automaton/neighborhoods/NeighborhoodFactory.hpp>

#include <cstdlib>


namespace cellular_automaton
{
    namespace
    {
        Neighborhood makeBoxNeighborhood(int size)
        {
            Neighborhood n;
            n.offsets.reserve(static_cast<size_t>(((2 * size + 1) * (2 * size + 1)) - 1));

            for (int dy = -size; dy <= size; ++dy) {
                for (int dx = -size; dx <= size; ++dx)
                {
                    if (dx == 0 && dy == 0) {
                        continue;
}

                    n.offsets.emplace_back(dx, dy);
                }
}

            return n;
        }

        Neighborhood makeDiamondNeighborhood(int size)
        {
            Neighborhood n;
            n.offsets.reserve(static_cast<size_t>(2 * size * (size + 1)));

            for (int dy = -size; dy <= size; ++dy)
            {
                const int span = size - std::abs(dy);
                for (int dx = -span; dx <= span; ++dx)
                {
                    if (dx == 0 && dy == 0) {
                        continue;
}

                    n.offsets.emplace_back(dx, dy);
                }
            }

            return n;
        }
    }

    Neighborhood NeighborhoodFactory::create(NeighborhoodShape shape, int size)
    {
        if (size <= 0) {
            return {};
}

        switch (shape)
        {
            case NeighborhoodShape::Box:     return makeBoxNeighborhood(size);
            case NeighborhoodShape::Diamond: return makeDiamondNeighborhood(size);
        }

        return {};
    }
}
