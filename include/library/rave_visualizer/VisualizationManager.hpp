#ifndef EDNA_ENGINE_VISUALIZATIONMANAGER_HPP
#define EDNA_ENGINE_VISUALIZATIONMANAGER_HPP

namespace RaveVisualizer
{
    class VisualizationManager
    {
    public:
        void TryChangeType(VisualizationType new_type); // check if already fading, activate new visual renderer, start fade to it

    private:
        RaveState rave_state;
    };
} // RaveVisualizer

#endif //EDNA_ENGINE_VISUALIZATIONMANAGER_HPP