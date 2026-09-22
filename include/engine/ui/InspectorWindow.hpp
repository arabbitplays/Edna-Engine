#ifndef INSPECTORWINDOW_HPP
#define INSPECTORWINDOW_HPP

#include "SceneManager.hpp"

#include <GuiWindow.hpp>
#include <memory>
#include <Node.hpp>
#include <VulkanContext.hpp>

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
