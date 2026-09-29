#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP
#include <engine/ui/core/GuiWindow.hpp>

namespace RtEngine
{
    class GameWindow final : public GuiWindow
    {
    public:
        GameWindow() = default;
        ~GameWindow() override = default;

        void createFrame() override;
    };
} // namespace RtEngine
#endif // GAMEWINDOW_HPP
