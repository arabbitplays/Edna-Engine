#ifndef EDNA_ENGINE_COMPOSITIONMANAGER_HPP
#define EDNA_ENGINE_COMPOSITIONMANAGER_HPP

#include "RaveState.hpp"
#include "VisualizationType.hpp"

namespace RaveVisualizer
{
    class CompositionManager
    {
    public:
        void TryChangeType(VisualizationType new_type);

        const RaveState& state() const { return rave_state; }

    private:
        RaveState rave_state;
    };
} // RaveVisualizer

#endif //EDNA_ENGINE_COMPOSITIONMANAGER_HPP
