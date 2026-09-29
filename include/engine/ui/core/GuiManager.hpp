//
// Created by oschdi on 25.01.26.
//

#ifndef VULKAN_RAYTRACING_GUIMANAGER_HPP
#define VULKAN_RAYTRACING_GUIMANAGER_HPP
#include <engine/ui/windows/AssetsWindow.hpp>
#include <engine/ui/windows/GameWindow.hpp>
#include <engine/ui/windows/HierarchyWindow.hpp>
#include <engine/ui/windows/InspectorWindow.hpp>
#include <engine/scene_graph/SceneManager.hpp>

#include "engine/ui/windows/MainWindow.hpp"

namespace RtEngine
{
    class GuiManager
    {
    public:
        GuiManager(
            const std::shared_ptr<SceneManager>& scene_manager, const std::shared_ptr<GuiRenderer>& gui_renderer);

        void addCallbackToAll(const std::function<void(const UpdateFlagsHandle&)>& callback) const;

        std::shared_ptr<GuiRenderer> gui_renderer;

        std::shared_ptr<MainWindow> main_window;
        std::shared_ptr<OptionsWindow> options_window;
        std::shared_ptr<InspectorWindow> inspector_window;
        std::shared_ptr<HierarchyWindow> hierarchy_window;
        std::shared_ptr<AssetsWindow> assets_window;
        std::shared_ptr<GameWindow> game_window;

    private:
        std::shared_ptr<SceneManager> scene_manager;
    };
} // namespace RtEngine

#endif // VULKAN_RAYTRACING_GUIMANAGER_HPP