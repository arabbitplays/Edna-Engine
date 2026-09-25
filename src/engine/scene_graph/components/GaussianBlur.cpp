#include "GaussianBlur.hpp"

#include "compute/GaussianBlurRenderer.hpp"
#include "EngineContext.hpp"
#include "Node.hpp"
#include "Scene.hpp"

#include <format>
#include <logging/LogManager.hpp>

namespace RtEngine
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<GaussianBlur>();
            return instance;
        }
    } // namespace

    GaussianBlur::GaussianBlur() = default;
    GaussianBlur::GaussianBlur(const std::shared_ptr<EngineContext>& context, const std::shared_ptr<Node>& node)
        : Component(context, node)
    {
    }
    GaussianBlur::~GaussianBlur() = default;

    void GaussianBlur::OnUpdate()
    {
        if ((!horizontal_renderer || !vertical_renderer) && !tryInitialize())
        {
            return;
        }
        if (!update_gate.tick())
        {
            return;
        }

        const uint32_t effective_radius = active ? radius : 0u;
        horizontal_renderer->setRadius(effective_radius);
        vertical_renderer->setRadius(effective_radius);
    }

    void GaussianBlur::OnDestroy()
    {
        if (resize_callback_handle != 0 && context && context->swapchain_manager)
        {
            context->swapchain_manager->removeRecreateCallback(resize_callback_handle);
            resize_callback_handle = 0;
        }
    }

    bool GaussianBlur::tryInitialize()
    {
        if (!context || !context->scene_manager)
        {
            return false;
        }

        const auto scene = context->scene_manager->getCurrentScene();
        if (!scene)
        {
            return false;
        }

        const auto it = scene->nodes.find(source_node);
        if (it == scene->nodes.end())
        {
            logger()->warn(std::format("GaussianBlur: source node '{}' not found in scene", source_node));
            return false;
        }

        std::shared_ptr<ImageConnector> input;
        for (const auto& comp : it->second->components)
        {
            if (auto out = comp->getOutputConnector())
            {
                input = std::move(out);
                break;
            }
        }
        if (!input)
        {
            return false;
        }

        const auto rendering_manager = context->rendering_manager;
        const auto vulkan_context = rendering_manager->getVulkanContext();
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        horizontal_renderer = std::make_shared<GaussianBlurRenderer>(
            vulkan_context, extent, input, GaussianBlurRenderer::Direction::Horizontal);
        horizontal_renderer->init();
        rendering_manager->addComputeRenderer(horizontal_renderer, nullptr);

        vertical_renderer = std::make_shared<GaussianBlurRenderer>(vulkan_context, extent,
            horizontal_renderer->getOutputConnector(), GaussianBlurRenderer::Direction::Vertical);
        vertical_renderer->init();
        rendering_manager->addComputeRenderer(vertical_renderer, nullptr);

        resize_callback_handle = context->swapchain_manager->addRecreateCallback(
            [this](uint32_t width, uint32_t height)
            {
                const VkExtent2D new_extent{width, height};
                if (horizontal_renderer)
                {
                    horizontal_renderer->handleResize(new_extent);
                }
                if (vertical_renderer)
                {
                    vertical_renderer->handleResize(new_extent);
                }
            });

        return true;
    }

    std::shared_ptr<ImageConnector> GaussianBlur::getOutputConnector() const
    {
        return vertical_renderer ? vertical_renderer->getOutputConnector() : nullptr;
    }

    void GaussianBlur::initProperties(
        const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& /*update_flags*/)
    {
        if (config->startChild(COMPONENT_NAME))
        {
            config->addString("source_node", &source_node);
            config->addUint("radius", &radius, MIN_RADIUS, MAX_RADIUS);
            config->addBool("active", &active);
            config->endChild();
        }
    }
} // namespace RtEngine
