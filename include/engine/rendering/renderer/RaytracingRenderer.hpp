#ifndef BASICS_VULKANENGINE_HPP
#define BASICS_VULKANENGINE_HPP

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define TINYOBJLOADER_IMPLEMENTATION

#include "MeshRepository.hpp"
#include "Renderer.hpp"
#include "UpdateFlagValue.hpp"

#include <../rendering/vulkan_scene_representation/SceneAdapter.hpp>
#include <GuiRenderer.hpp>
#include <GuiWindow.hpp>
#include <memory>
#include <RenderTarget.hpp>
#include <VulkanContext.hpp>

namespace RtEngine
{

    class RaytracingRenderer : public ISerializable, public Renderer
    {
    public:
        RaytracingRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
            const std::shared_ptr<MeshRepository>& mesh_repository,
            const std::shared_ptr<TextureRepository>& texture_repository, const uint32_t max_frames_in_flight);

        void init() override;
        void initProperties(const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& update_flags) override;

        void loadScene(const std::shared_ptr<IScene>& scene);

        void writeResources(
            const std::shared_ptr<DrawContext>& draw_context, UpdateFlagsHandle update_flags, uint32_t frame_idx);
        void writeRenderTarget(const std::shared_ptr<RenderTarget>& target);

        VkCommandBuffer recordCommandBuffer(uint32_t frame_idx) override;
        QueueType queueType() const override
        {
            return GRAPHICS;
        }

        void outputRenderingTarget(const std::shared_ptr<RenderTarget>& target, const std::string& output_path);
        float* downloadRenderTarget(const std::shared_ptr<RenderTarget>& target) const;
        static uint8_t* fixImageFormatForStorage(void* image_data, size_t pixel_count, VkFormat originalFormat);

        std::unordered_map<std::string, std::shared_ptr<Material>> getMaterials() const;

    protected:
        uint32_t recursion_depth = 5;
        std::vector<int32_t> push_constants{};

        std::shared_ptr<MeshRepository> mesh_repository;
        std::shared_ptr<TextureRepository> texture_repository;

        std::shared_ptr<SceneAdapter> scene_adapter;
        std::shared_ptr<RenderTarget> current_target;

        static bool hasStencilComponent(VkFormat format);

        void recordRenderToImage(VkCommandBuffer commandBuffer, uint32_t frame_idx);

        void* createPushConstants(uint32_t* size, const std::shared_ptr<RenderTarget>& target);
    };

} // namespace RtEngine
#endif // BASICS_VULKANENGINE_HPP
