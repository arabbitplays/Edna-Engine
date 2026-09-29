#ifndef INSPECTORWINDOW_HPP
#define INSPECTORWINDOW_HPP

#include <engine/scene_graph/SceneManager.hpp>

#include <engine/ui/core/GuiWindow.hpp>
#include <memory>
#include <engine/scene_graph/Node.hpp>
#include <engine/rendering/context/VulkanContext.hpp>

namespace RtEngine
{
    class InspectorWindow : public GuiWindow
    {
    public:
        InspectorWindow() = default;
        explicit InspectorWindow(const std::shared_ptr<SceneManager>& scene);
        ~InspectorWindow() override = default;

        void createFrame() override;
        void setNode(const std::shared_ptr<Node>& node);

    private:
        std::shared_ptr<SceneManager> scene_manager;
        std::shared_ptr<Node> node;
    };

} // namespace RtEngine
#endif // INSPECTORWINDOW_HPP
