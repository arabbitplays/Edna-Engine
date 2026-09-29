#ifndef ASSETSWINDOW_HPP
#define ASSETSWINDOW_HPP
#include <engine/ui/core/GuiWindow.hpp>

namespace RtEngine
{
    class AssetsWindow final : public GuiWindow
    {
    public:
        AssetsWindow() = default;
        ~AssetsWindow() override = default;

        void createFrame() override;
    };
} // namespace RtEngine
#endif // ASSETSWINDOW_HPP
