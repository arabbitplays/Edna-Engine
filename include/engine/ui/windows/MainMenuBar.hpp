#ifndef EDNA_ENGINE_MAINMENUBAR_HPP
#define EDNA_ENGINE_MAINMENUBAR_HPP
#include "engine/ui/core/GuiWindow.hpp"

namespace RtEngine
{
    class MainMenuBar : public GuiWindow
    {
    public:
        void createFrame() override;
    };
} // RtEngine

#endif //EDNA_ENGINE_MAINMENUBAR_HPP