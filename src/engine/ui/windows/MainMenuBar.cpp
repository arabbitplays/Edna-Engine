#include "../../../../include/engine/ui/windows/MainMenuBar.hpp"

#include "imgui.h"

namespace RtEngine
{
    void MainMenuBar::createFrame()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Project"))
            {
                ImGui::MenuItem("New");
                ImGui::MenuItem("Open");
                ImGui::MenuItem("Save");

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
} // RtEngine