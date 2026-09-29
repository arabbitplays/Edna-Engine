#include <engine/ui/windows/AssetsWindow.hpp>

#include <engine/ui/core/ImGuiIdProvider.hpp>

#include <imgui.h>

namespace RtEngine
{
    void AssetsWindow::createFrame()
    {
        if (!show_window)
        {
            return;
        }

        ImGui::Begin(ImGuiIdProvider::getName(UiElement::Assets), &show_window);
        ImGui::TextUnformatted("Assets (placeholder)");
        ImGui::End();
    }
} // namespace RtEngine
