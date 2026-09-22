#ifndef VULKAN_RAYTRACING_RENDERINGMANAGER_HPP
#define VULKAN_RAYTRACING_RENDERINGMANAGER_HPP
#include <memory>

#include "MeshRepository.hpp"
#include "PresentStage.hpp"
#include "RendererStack.hpp"
#include "SwapchainManager.hpp"
#include "SyncManager.hpp"
#include "TextureRepository.hpp"
#include "VulkanContext.hpp"
#include "RaytracingRenderer.hpp"
#include "compute/ComputeRenderer.hpp"

namespace RtEngine {
    class RenderingManager {
    public:
        RenderingManager() = default;
        RenderingManager(const std::shared_ptr<Window> &window, std::string resources_dir,
                         bool enable_validation_layer, bool enable_raytracing = true);

        void initRendererProperties(const std::shared_ptr<IProperties> &properties, const std::shared_ptr<UpdateFlags> &update_flags);

        bool raytracingEnabled() const { return enable_raytracing; }

        std::shared_ptr<VulkanContext> getVulkanContext() const;
        std::shared_ptr<RaytracingRenderer> getRaytracingRenderer() const;
        std::shared_ptr<GuiRenderer> getGuiRenderer() const;
        std::shared_ptr<PresentStage> getPresentStage() const;
        std::shared_ptr<RendererStack> getRendererStack() const;
        std::shared_ptr<SwapchainManager> getSwapchainManager() const;
        std::shared_ptr<SyncManager> getSyncManager() const;
        std::shared_ptr<MeshRepository> getMeshRepository() const;
        std::shared_ptr<TextureRepository> getTextureRepository() const;

        std::shared_ptr<RenderTarget> createRenderTarget(uint32_t width, uint32_t height);

        void addComputeRenderer(const std::shared_ptr<ComputeRenderer>& renderer,
                                std::shared_ptr<ImageConnector> new_present_connector = nullptr);

        bool framebufferWasResized();

        void destroy();
    private:
        void createVulkanContext();
        std::shared_ptr<DescriptorAllocator> createDescriptorAllocator() const;

        void createRenderer();
        void createRepositories();

        std::shared_ptr<Window> window;
        bool validation_layers_enabled;
        bool enable_raytracing = true;
        std::string resources_dir;
        uint32_t max_frames_in_flight = 1;

        DeletionQueue deletion_queue;
        std::shared_ptr<VulkanContext> vulkan_context;
        std::shared_ptr<SwapchainManager> swapchain_manager;
        std::shared_ptr<SyncManager> sync_manager;

        std::shared_ptr<MeshRepository> mesh_repository;
        std::shared_ptr<TextureRepository> texture_repository;

        std::shared_ptr<RaytracingRenderer> raytracing_renderer;
        std::shared_ptr<GuiRenderer> gui_renderer;
        std::shared_ptr<PresentStage> present_stage;
        std::shared_ptr<RendererStack> renderer_stack;
        std::shared_ptr<ImageConnector> rt_target_connector;

		bool framebufferResized = false;

    };
} // RtEngine

#endif //VULKAN_RAYTRACING_RENDERINGMANAGER_HPP