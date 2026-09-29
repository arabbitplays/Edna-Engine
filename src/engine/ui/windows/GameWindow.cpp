#include <engine/ui/windows/GameWindow.hpp>

#include <engine/ui/core/ImGuiIdProvider.hpp>

#include <imgui.h>

namespace RtEngine
{
    void GameWindow::createFrame()
    {
        if (!show_window)
        {
            return;
        }

        ImGui::Begin(ImGuiIdProvider::getName(UiElement::Game), &show_window);
        ImGui::TextUnformatted("Game (placeholder)");
        ImGui::End();
    }
} // namespace RtEngine
