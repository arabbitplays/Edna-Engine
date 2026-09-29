#include <engine/ui/windows/MainWindow.hpp>

#include <engine/ui/core/ImGuiIdProvider.hpp>

#include <imgui.h>
#include <imgui_internal.h>

namespace RtEngine
{
    void MainWindow::createFrame()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoDocking;

        ImGui::Begin(ImGuiIdProvider::getName(UiElement::MainWindow), nullptr, flags);

        if (!dockspace_initialized)
        {
            setupDockspace();
            dockspace_initialized = true;
        }

        ImGui::DockSpace(ImGuiIdProvider::getId(UiElement::MainDockSpace));

        ImGui::End();
    }

    void MainWindow::setupDockspace()
    {
        ImGuiID dockspace_id = ImGuiIdProvider::getId(UiElement::MainDockSpace);

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

        ImGuiID top;
        ImGuiID bottom;
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, &bottom, &top);

        ImGuiID left;
        ImGuiID top_rest;
        ImGui::DockBuilderSplitNode(top, ImGuiDir_Left, 0.2f, &left, &top_rest);

        ImGuiID right;
        ImGuiID center;
        ImGui::DockBuilderSplitNode(top_rest, ImGuiDir_Right, 0.25f, &right, &center);

        ImGui::DockBuilderDockWindow(ImGuiIdProvider::getName(UiElement::Hierarchy), left);
        ImGui::DockBuilderDockWindow(ImGuiIdProvider::getName(UiElement::Inspector), right);
        ImGui::DockBuilderDockWindow(ImGuiIdProvider::getName(UiElement::Game), center);
        ImGui::DockBuilderDockWindow(ImGuiIdProvider::getName(UiElement::Assets), bottom);
        ImGui::DockBuilderDockWindow(ImGuiIdProvider::getName(UiElement::Options), bottom);

        ImGui::DockBuilderFinish(dockspace_id);
    }
} // RtEngine
