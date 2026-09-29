#ifndef EDNA_ENGINE_MAINWINDOW_H
#define EDNA_ENGINE_MAINWINDOW_H
#include "engine/ui/core/GuiWindow.hpp"

namespace RtEngine
{
    class MainWindow : public GuiWindow
    {
    public:
        void createFrame() override;

    private:
        void setupDockspace();

        bool dockspace_initialized = false;
    };
} // RtEngine

#endif //EDNA_ENGINE_MAINWINDOW_H