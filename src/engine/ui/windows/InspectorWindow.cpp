#include <engine/ui/windows/InspectorWindow.hpp>

#include <engine/ui/core/ImGuiIdProvider.hpp>
#include <properties/ImGuiProperties.hpp>
#include <engine/rendering/vulkan_scene_representation/SceneAdapter.hpp>
#include <engine/UpdateFlagValue.hpp>

#include <imgui.h>

namespace RtEngine
{
    InspectorWindow::InspectorWindow(const std::shared_ptr<SceneManager>& scene_manager) : scene_manager(scene_manager)
    {
    }

    void InspectorWindow::createFrame()
    {
        if (!node || !scene_manager)
        {
            return;
        }

        std::shared_ptr<Node> root_node = scene_manager->getCurrentScene()->nodes["root"];

        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);

        auto update_flags = std::make_shared<UpdateFlags>();
        if (show_window)
        {
            std::shared_ptr<ImGuiProperties> props = std::make_shared<ImGuiProperties>();

            ImGui::Begin(ImGuiIdProvider::getName(UiElement::Inspector), &show_window);
            ImGui::Text("%s", node->name.c_str());
            ImGui::Separator();

            for (auto& component : node->components)
            {
                component->initProperties(props, update_flags);
            }

            ImGui::End();
        }

        if (update_flags->hasAny())
        {
            notifyUpdate(update_flags);
        }
    }

    void InspectorWindow::setNode(const std::shared_ptr<Node>& node)
    {
        this->node = node;
    }

} // namespace RtEngine
