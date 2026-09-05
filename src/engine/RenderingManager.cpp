#include "../../include/engine/RenderingManager.hpp"

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
            present_stage->cleanup();
            gui_renderer->cleanup();
            glitch_renderer->cleanup();
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
        raytracing_renderer = std::make_shared<RaytracingRenderer>(vulkan_context, resources_dir, max_frames_in_flight);
        raytracing_renderer->init();
        gui_renderer = std::make_shared<GuiRenderer>(vulkan_context);
        present_stage = std::make_shared<PresentStage>(vulkan_context, sync_manager, gui_renderer, max_frames_in_flight);
        present_stage->init();

        // Renderer stages + present stage.
        sync_manager->setStagesPerFrame(2);
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
        return std::make_shared<RenderTarget>(vulkan_context->resource_builder, extent, max_frames_in_flight);
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