#include "../../include/engine/RenderingManager.hpp"

#include "compute/ComputeRenderer.hpp"
#include "compute/GlitchRenderer.hpp"

namespace RtEngine {
    RenderingManager::RenderingManager(const std::shared_ptr<Window> &window, std::string resources_dir, const bool enable_validation_layer)
        : window(window), validation_layers_enabled(enable_validation_layer), resources_dir(resources_dir) {

        createVulkanContext();
        createRenderer();

		window->addResizeCallback([this](uint32_t width, uint32_t height) {
			framebufferResized = true;
		});
    }


    void RenderingManager::initRendererProperties(const std::shared_ptr<IProperties> &properties, const std::shared_ptr<UpdateFlags> &update_flags) {
        raytracing_renderer->initProperties(properties, update_flags);
    }

    void RenderingManager::createVulkanContext() {
        vulkan_context = std::make_shared<VulkanContext>();

        vulkan_context->window = window;
        vulkan_context->device_manager = std::make_shared<DeviceManager>(window->getHandle(), validation_layers_enabled);

        vulkan_context->command_manager = std::make_shared<CommandManager>(vulkan_context->device_manager);
        vulkan_context->resource_builder =
                std::make_shared<ResourceBuilder>(vulkan_context->device_manager, vulkan_context->command_manager,
                                                  resources_dir);
        vulkan_context->swapchain =
                std::make_shared<Swapchain>(vulkan_context->device_manager, window->getHandle(), vulkan_context->resource_builder);
        vulkan_context->descriptor_allocator = createDescriptorAllocator();

        swapchain_manager = std::make_shared<SwapchainManager>(vulkan_context->swapchain);
        sync_manager = std::make_shared<SyncManager>(max_frames_in_flight,
                                                     vulkan_context->device_manager,
                                                     swapchain_manager);

        deletion_queue.pushFunction([&]() {
            vulkan_context->descriptor_allocator->destroyPools(vulkan_context->device_manager->getDevice());
            raytracing_renderer->cleanup();
            for (const auto& renderer : renderer_stack->getRenderers()) {
                if (auto compute = std::dynamic_pointer_cast<ComputeRenderer>(renderer)) {
                    compute->cleanup();
                }
            }
            if (rt_target_connector) {
                rt_target_connector->destroy();
            }
            present_stage->cleanup();
            gui_renderer->cleanup();
            sync_manager->destroy();
            vulkan_context->swapchain->destroy();
            vulkan_context->command_manager->destroy();
            vulkan_context->device_manager->destroy();
        });
    }

    std::shared_ptr<DescriptorAllocator> RenderingManager::createDescriptorAllocator() const {
        std::vector<DescriptorAllocator::PoolSizeRatio> poolRatios = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
            {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
    };

        auto descriptorAllocator = std::make_shared<DescriptorAllocator>();
        descriptorAllocator->init(vulkan_context->device_manager->getDevice(), 8, poolRatios);

        return descriptorAllocator;
    }

    void RenderingManager::createRenderer() {
        VkExtent2D extent = vulkan_context->swapchain->extent;

        raytracing_renderer = std::make_shared<RaytracingRenderer>(vulkan_context, resources_dir, max_frames_in_flight);
        raytracing_renderer->init();
        gui_renderer = std::make_shared<GuiRenderer>(vulkan_context);
        present_stage = std::make_shared<PresentStage>(vulkan_context, sync_manager, gui_renderer, max_frames_in_flight);
        present_stage->init();

        rt_target_connector = std::make_shared<ImageConnector>(
            vulkan_context->resource_builder, extent, max_frames_in_flight,
            VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        auto glitch_renderer = std::make_shared<GlitchRenderer>(
            vulkan_context, extent, rt_target_connector, max_frames_in_flight);
        glitch_renderer->init();

        renderer_stack = std::make_shared<RendererStack>();
        renderer_stack->addRenderer(raytracing_renderer);
        renderer_stack->addRenderer(glitch_renderer);
        renderer_stack->setPresentStage(present_stage);
        renderer_stack->setPresentConnector(glitch_renderer->getOutputConnector());

        sync_manager->setStagesPerFrame(static_cast<uint32_t>(renderer_stack->getRenderers().size()) + 1);
    }

    std::shared_ptr<VulkanContext> RenderingManager::getVulkanContext() const {
        assert(vulkan_context != nullptr);
        return vulkan_context;
    }

    std::shared_ptr<RaytracingRenderer> RenderingManager::getRaytracingRenderer() const {
        assert(raytracing_renderer != nullptr);
        return raytracing_renderer;
    }

    std::shared_ptr<GuiRenderer> RenderingManager::getGuiRenderer() const {
        assert(gui_renderer != nullptr);
        return gui_renderer;
    }

    std::shared_ptr<PresentStage> RenderingManager::getPresentStage() const {
        assert(present_stage != nullptr);
        return present_stage;
    }

    std::shared_ptr<RendererStack> RenderingManager::getRendererStack() const {
        assert(renderer_stack != nullptr);
        return renderer_stack;
    }

    std::shared_ptr<SwapchainManager> RenderingManager::getSwapchainManager() const {
        assert(swapchain_manager != nullptr);
        return swapchain_manager;
    }

    std::shared_ptr<SyncManager> RenderingManager::getSyncManager() const {
        assert(sync_manager != nullptr);
        return sync_manager;
    }

    std::shared_ptr<RenderTarget> RenderingManager::createRenderTarget(uint32_t width, uint32_t height) {
        VkExtent2D extent(width, height);
        rt_target_connector->recreate(extent);
        return std::make_shared<RenderTarget>(vulkan_context->resource_builder, extent, max_frames_in_flight,
                                              rt_target_connector);
    }

    bool RenderingManager::framebufferWasResized()
    {
        bool result = framebufferResized;
        framebufferResized = false;
        return result;
    }

    void RenderingManager::destroy() {
        deletion_queue.flush();
    }
} // RtEngine
