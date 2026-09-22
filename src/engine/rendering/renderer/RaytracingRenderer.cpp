#include "ImageUtil.hpp"
#include "QuickTimer.hpp"
#include "UpdateFlagValue.hpp"

#include <../../../include/engine/rendering/renderer/RaytracingRenderer.hpp>
#include <cstdlib>
#include <filesystem>
#include <glm/gtc/packing.hpp>
#include <HierarchyWindow.hpp>
#include <logging/LogManager.hpp>
#include <PathUtil.hpp>
#include <RandomUtil.hpp>
#include <SceneWriter.hpp>
#include <set>
#include <utility>

namespace RtEngine
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<RaytracingRenderer>();
            return instance;
        }
    } // namespace

    void cmdTraceRaysKhr(VkDevice device, VkCommandBuffer command_buffer,
        const VkStridedDeviceAddressRegionKHR* p_raygen_shader_binding_table,
        const VkStridedDeviceAddressRegionKHR* p_miss_shader_binding_table,
        const VkStridedDeviceAddressRegionKHR* p_hit_shader_binding_table,
        const VkStridedDeviceAddressRegionKHR* p_callable_shader_binding_table, uint32_t width, uint32_t height,
        uint32_t depth)
    {
        auto func = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR");
        if (func != nullptr)
        {
            func(command_buffer, p_raygen_shader_binding_table, p_miss_shader_binding_table, p_hit_shader_binding_table,
                p_callable_shader_binding_table, width, height, depth);
            return;
        }
    }

    RaytracingRenderer::RaytracingRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
        const std::shared_ptr<MeshRepository>& mesh_repository,
        const std::shared_ptr<TextureRepository>& texture_repository, const uint32_t max_frames_in_flight)
        : Renderer(vulkan_context, max_frames_in_flight), mesh_repository(mesh_repository),
          texture_repository(texture_repository)
    {
    }

    void RaytracingRenderer::init()
    {
        scene_adapter = std::make_shared<SceneAdapter>(
            vulkan_context, texture_repository, max_frames_in_flight, DeviceManager::RAYTRACING_PROPERTIES);
        deletion_queue.pushFunction([&]() { scene_adapter->clearResources(); });

        Renderer::init();
    }

    bool RaytracingRenderer::hasStencilComponent(const VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void RaytracingRenderer::loadScene(const std::shared_ptr<IScene>& scene)
    {
        scene_adapter->loadNewScene(scene);
    }

    void RaytracingRenderer::writeResources(
        const std::shared_ptr<DrawContext>& draw_context, UpdateFlagsHandle update_flags, uint32_t frame_idx)
    {
        scene_adapter->updateScene(draw_context, frame_idx, std::move(update_flags));
    }

    void RaytracingRenderer::writeRenderTarget(const std::shared_ptr<RenderTarget>& target)
    {
        current_target = target;
        scene_adapter->updateRenderTarget(target);
    }

    VkCommandBuffer RaytracingRenderer::recordCommandBuffer(uint32_t frame_idx)
    {
        VkCommandBuffer cmd = getFreshCommandBuffer(frame_idx);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("RaytracingRenderer: failed to begin command buffer");
        }

        recordRenderToImage(cmd, frame_idx);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        {
            throw std::runtime_error("RaytracingRenderer: failed to end command buffer");
        }
        return cmd;
    }

    void RaytracingRenderer::recordRenderToImage(VkCommandBuffer command_buffer, uint32_t frame_idx)
    {
        const std::shared_ptr<RenderTarget>& target = current_target;
        RaytracingPipeline pipeline = *scene_adapter->getMaterial()->pipeline;

        const uint32_t handle_size_aligned =
            VulkanUtil::alignedSize(DeviceManager::RAYTRACING_PROPERTIES.shaderGroupHandleSize,
                DeviceManager::RAYTRACING_PROPERTIES.shaderGroupHandleAlignment);

        VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry{};
        raygen_shader_sbt_entry.deviceAddress = pipeline.raygenShaderBindingTable.deviceAddress;
        raygen_shader_sbt_entry.stride = handle_size_aligned;
        raygen_shader_sbt_entry.size = handle_size_aligned;

        VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry{};
        miss_shader_sbt_entry.deviceAddress = pipeline.missShaderBindingTable.deviceAddress;
        miss_shader_sbt_entry.stride = handle_size_aligned;
        miss_shader_sbt_entry.size = handle_size_aligned;

        VkStridedDeviceAddressRegionKHR closest_hit_shader_sbt_entry{};
        closest_hit_shader_sbt_entry.deviceAddress = pipeline.hitShaderBindingTable.deviceAddress;
        closest_hit_shader_sbt_entry.stride = handle_size_aligned;
        closest_hit_shader_sbt_entry.size = handle_size_aligned;

        VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry{};

        std::vector<VkDescriptorSet> descriptor_sets{};
        descriptor_sets.push_back(scene_adapter->getSceneDescriptorSet(frame_idx));
        descriptor_sets.push_back(scene_adapter->getMaterial()->materialDescriptorSet);

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.getHandle());
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.getLayoutHandle(), 0,
            static_cast<uint32_t>(descriptor_sets.size()), descriptor_sets.data(), 0, nullptr);

        uint32_t pc_size;
        void* pc_data = createPushConstants(&pc_size, target);
        vkCmdPushConstants(command_buffer, pipeline.getLayoutHandle(),
            VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, 0,
            pc_size, pc_data);

        const auto [width, height] = target->getExtent();
        cmdTraceRaysKhr(vulkan_context->device_manager->getDevice(), command_buffer, &raygen_shader_sbt_entry,
            &miss_shader_sbt_entry, &closest_hit_shader_sbt_entry, &callable_shader_sbt_entry, width, height, 1);
    }

    void* RaytracingRenderer::createPushConstants(uint32_t* size, const std::shared_ptr<RenderTarget>& target)
    {
        push_constants.clear();

        push_constants.push_back(recursion_depth);
        std::shared_ptr<Material> material = scene_adapter->getMaterial();
        material->getPushConstantValues(push_constants);

        push_constants.push_back(target->getAccumulatedFrameCount());
        push_constants.push_back(target->getSamplesPerFrame());

        *size = sizeof(uint32_t) * push_constants.size();
        return push_constants.data();
    }

    float* RaytracingRenderer::downloadRenderTarget(const std::shared_ptr<RenderTarget>& target) const
    {
        AllocatedImage image = target->getLastTargetImage();
        uint8_t* data = vulkan_context->resource_builder->downloadImage(image, sizeof(float));
        return reinterpret_cast<float*>(data);
    }

    void RaytracingRenderer::outputRenderingTarget(
        const std::shared_ptr<RenderTarget>& target, const std::string& output_path)
    {
        QuickTimer timer("Output render target");

        AllocatedImage render_target = target->getLastTargetImage();
        uint8_t* data = vulkan_context->resource_builder->downloadImage(render_target, sizeof(uint32_t));
        uint8_t* fixed_data = fixImageFormatForStorage(
            data, render_target.imageExtent.width * render_target.imageExtent.height, render_target.imageFormat);

        ImageUtil::writePNG(output_path, fixed_data, render_target.imageExtent.width, render_target.imageExtent.height);

        delete[] fixed_data;
    }

    // target format is R8G8B8A8_UNORM
    uint8_t* RaytracingRenderer::fixImageFormatForStorage(void* data, size_t pixel_count, VkFormat original_format)
    {

        if (original_format == VK_FORMAT_R8G8B8A8_UNORM)
        {
            return static_cast<uint8_t*>(data);
        }

        if (original_format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            auto* image_data = static_cast<uint8_t*>(data);
#pragma omp parallel for
            for (size_t i = 0; i < pixel_count; i++)
            {
                std::swap(image_data[i * 4], image_data[(i * 4) + 2]); // Swap B (0) and R (2)
            }
            return image_data;
        }
        if (original_format == VK_FORMAT_R32G32B32A32_SFLOAT)
        {
            auto* output_image = new uint8_t[pixel_count * 4];
            auto* image_data = static_cast<float*>(data);

#pragma omp parallel for
            for (size_t i = 0; i < pixel_count * 4; i++)
            {
                // Clamp each channel to the [0, 1] range and then scale to [0, 255]
                output_image[i] = static_cast<uint8_t>(std::fmin(1.0F, std::fmax(0.0F, image_data[i])) * 255);
            }
            delete[] image_data;
            return output_image;
        }
        logger()->error("Image format of the storage image is not supported to be stored correctly!");
        return nullptr;
    }

    void RaytracingRenderer::initProperties(
        const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& update_flags)
    {
        if (config->startChild("renderer"))
        {
            if (config->addUint("recursion_depth", &recursion_depth, 1, 10))
            {
                update_flags->setFlag(TARGET_RESET);
            }
            config->endChild();
        }

        for (const auto& [name, material] : scene_adapter->defaultMaterials)
        {
            material->initProperties(config, update_flags);
        }
    }

    std::unordered_map<std::string, std::shared_ptr<Material>> RaytracingRenderer::getMaterials() const
    {
        return scene_adapter->defaultMaterials;
    }
} // namespace RtEngine
