#include <engine/ui/core/GuiManager.hpp>

#include "engine/ui/windows/MainMenuBar.hpp"
#include "engine/ui/windows/MainWindow.hpp"

namespace RtEngine
{
    GuiManager::GuiManager(
        const std::shared_ptr<SceneManager>& scene_manager, const std::shared_ptr<GuiRenderer>& gui_renderer)
        : gui_renderer(gui_renderer)
    {
        auto main_menu = std::make_shared<MainMenuBar>();
        gui_renderer->addWindow(main_menu);

        main_window = std::make_shared<MainWindow>();
        gui_renderer->addWindow(main_window);

        options_window = std::make_shared<OptionsWindow>();
        gui_renderer->addWindow(options_window);

        inspector_window = std::make_shared<InspectorWindow>(scene_manager);
        gui_renderer->addWindow(inspector_window);

        hierarchy_window = std::make_shared<HierarchyWindow>(inspector_window, scene_manager);
        gui_renderer->addWindow(hierarchy_window);

        game_window = std::make_shared<GameWindow>();
        gui_renderer->addWindow(game_window);

        assets_window = std::make_shared<AssetsWindow>();
        gui_renderer->addWindow(assets_window);
    }

    void GuiManager::addCallbackToAll(const std::function<void(const UpdateFlagsHandle&)>& callback) const
    {
        options_window->addCallback(callback);
        inspector_window->addCallback(callback);
        hierarchy_window->addCallback(callback);
    }
} // namespace RtEngine
