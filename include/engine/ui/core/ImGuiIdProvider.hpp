#ifndef IMGUIIDS_HPP
#define IMGUIIDS_HPP

#include <imgui.h>

namespace RtEngine
{
    enum class UiElement
    {
        MainWindow,
        MainDockSpace,
        Hierarchy,
        Inspector,
        Options,
        Assets,
        Game,
    };

    class ImGuiIdProvider
    {
    public:
        static inline const char* getName(UiElement id)
        {
            switch (id)
            {
                case UiElement::MainWindow:    return "MainWindow";
                case UiElement::MainDockSpace: return "MainDockSpace";
                case UiElement::Hierarchy:     return "Hierarchy";
                case UiElement::Inspector:     return "Inspector";
                case UiElement::Options:       return "Options";
                case UiElement::Assets:        return "Assets";
                case UiElement::Game:          return "Game";
            }
            return "";
        }

        static inline ImGuiID getId(UiElement id)
        {
            return ImGui::GetID(getName(id));
        }
    };
} // namespace RtEngine

#endif // IMGUIIDS_HPP
