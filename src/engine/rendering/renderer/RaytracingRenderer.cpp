#include <HierarchyWindow.hpp>
#include <SceneWriter.hpp>
#include <../../../include/engine/rendering/renderer/RaytracingRenderer.hpp>
#include <cstdlib>
#include <filesystem>
#include <PathUtil.hpp>
#include <set>
#include <RandomUtil.hpp>
#include <glm/gtc/packing.hpp>

#include "ImageUtil.hpp"
#include "UpdateFlagValue.hpp"

namespace RtEngine {

	void CmdTraceRaysKHR(VkDevice device, VkCommandBuffer commandBuffer,
						 const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
						 const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
						 const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
						 const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
						 uint32_t height, uint32_t depth) {
		auto func = (PFN_vkCmdTraceRaysKHR) vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR");
		if (func != nullptr) {
			return func(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable,
						pCallableShaderBindingTable, width, height, depth);
		}
	}

	RaytracingRenderer::RaytracingRenderer(const std::shared_ptr<VulkanContext> &vulkan_context,
		const std::string &resources_dir, const uint32_t max_frames_in_flight)
		: Renderer(vulkan_context, max_frames_in_flight), resources_dir(resources_dir) {
	}

	void RaytracingRenderer::init() {
		createRepositories();

		scene_adapter = std::make_shared<SceneAdapter>(vulkan_context, texture_repository, max_frames_in_flight,
													   DeviceManager::RAYTRACING_PROPERTIES);
		deletion_queue.pushFunction([&]() { scene_adapter->clearResources(); });

		Renderer::init();
	}

	void RaytracingRenderer::createRepositories() {
		mesh_repository = std::make_shared<MeshRepository>(vulkan_context, resources_dir);
		texture_repository = std::make_shared<TextureRepository>(vulkan_context->resource_builder);

		deletion_queue.pushFunction([&]() {
			mesh_repository->destroy();
			texture_repository->destroy();
		});
	}

	bool RaytracingRenderer::hasStencilComponent(const VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void RaytracingRenderer::loadScene(std::shared_ptr<IScene> scene) {
		scene_adapter->loadNewScene(scene);
	}

	void RaytracingRenderer::writeResources(const std::shared_ptr<DrawContext> &draw_context, UpdateFlagsHandle update_flags, uint32_t frame_idx) {
		scene_adapter->updateScene(draw_context, frame_idx, update_flags);
	}

	void RaytracingRenderer::writeRenderTarget(const std::shared_ptr<RenderTarget> &target) {
		current_target = target;
		scene_adapter->updateRenderTarget(target);
	}

	void RaytracingRenderer::waitForIdle() {
		vkDeviceWaitIdle(vulkan_context->device_manager->getDevice());
	}

	VkCommandBuffer RaytracingRenderer::recordCommandBuffer(uint32_t frame_idx) {
		VkCommandBuffer cmd = getFreshCommandBuffer(frame_idx);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("RaytracingRenderer: failed to begin command buffer");
		}

		recordRenderToImage(cmd, frame_idx);

		if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
			throw std::runtime_error("RaytracingRenderer: failed to end command buffer");
		}
		return cmd;
	}

	void RaytracingRenderer::recordRenderToImage(VkCommandBuffer commandBuffer, uint32_t frame_idx) {
		const std::shared_ptr<RenderTarget> &target = current_target;
		RaytracingPipeline pipeline = *scene_adapter->getMaterial()->pipeline;

		const uint32_t handleSizeAligned =
				VulkanUtil::alignedSize(DeviceManager::RAYTRACING_PROPERTIES.shaderGroupHandleSize,
										DeviceManager::RAYTRACING_PROPERTIES.shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
		raygenShaderSbtEntry.deviceAddress = pipeline.raygenShaderBindingTable.deviceAddress;
		raygenShaderSbtEntry.stride = handleSizeAligned;
		raygenShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
		missShaderSbtEntry.deviceAddress = pipeline.missShaderBindingTable.deviceAddress;
		missShaderSbtEntry.stride = handleSizeAligned;
		missShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR closestHitShaderSbtEntry{};
		closestHitShaderSbtEntry.deviceAddress = pipeline.hitShaderBindingTable.deviceAddress;
		closestHitShaderSbtEntry.stride = handleSizeAligned;
		closestHitShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

		std::vector<VkDescriptorSet> descriptor_sets{};
		descriptor_sets.push_back(scene_adapter->getSceneDescriptorSet(frame_idx));
		descriptor_sets.push_back(scene_adapter->getMaterial()->materialDescriptorSet);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.getHandle());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.getLayoutHandle(), 0,
								static_cast<uint32_t>(descriptor_sets.size()), descriptor_sets.data(), 0, nullptr);

		uint32_t pc_size;
		void *pc_data = createPushConstants(&pc_size, target);
		vkCmdPushConstants(commandBuffer, pipeline.getLayoutHandle(),
						   VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR |
								   VK_SHADER_STAGE_MISS_BIT_KHR,
						   0, pc_size, pc_data);

		const auto [width, height] = target->getExtent();
		CmdTraceRaysKHR(vulkan_context->device_manager->getDevice(), commandBuffer, &raygenShaderSbtEntry,
						&missShaderSbtEntry, &closestHitShaderSbtEntry, &callableShaderSbtEntry,
						width, height, 1);
	}

	void* RaytracingRenderer::createPushConstants(uint32_t* size, const std::shared_ptr<RenderTarget> &target) {
		push_constants.clear();

		push_constants.push_back(recursion_depth);
		std::shared_ptr<Material> material = scene_adapter->getMaterial();
		material->getPushConstantValues(push_constants);

		push_constants.push_back(target->getAccumulatedFrameCount());
		push_constants.push_back(target->getSamplesPerFrame());

		*size = sizeof(uint32_t) * push_constants.size();
		return push_constants.data();
	}

	float* RaytracingRenderer::downloadRenderTarget(const std::shared_ptr<RenderTarget> &target) const {
		AllocatedImage image = target->getLastTargetImage();
		uint8_t *data = vulkan_context->resource_builder->downloadImage(image, sizeof(float));
		return reinterpret_cast<float*>(data);
	}

	void RaytracingRenderer::outputRenderingTarget(const std::shared_ptr<RenderTarget> &target, const std::string &output_path) {
		QuickTimer timer("Output render target");

		AllocatedImage render_target = target->getLastTargetImage();
		uint8_t *data = vulkan_context->resource_builder->downloadImage(render_target, sizeof(uint32_t));
		uint8_t *fixed_data = fixImageFormatForStorage(
				data, render_target.imageExtent.width * render_target.imageExtent.height, render_target.imageFormat);

		ImageUtil::writePNG(output_path, fixed_data, render_target.imageExtent.width, render_target.imageExtent.height);

		delete[] fixed_data;
	}

	// target format is R8G8B8A8_UNORM
	uint8_t *RaytracingRenderer::fixImageFormatForStorage(void *data, size_t pixel_count, VkFormat originalFormat) {

		if (originalFormat == VK_FORMAT_R8G8B8A8_UNORM)
			return static_cast<uint8_t *>(data);

		if (originalFormat == VK_FORMAT_B8G8R8A8_UNORM) {
			auto image_data = static_cast<uint8_t *>(data);
#pragma omp parallel for
			for (size_t i = 0; i < pixel_count; i++) {
				std::swap(image_data[i * 4], image_data[i * 4 + 2]); // Swap B (0) and R (2)
			}
			return image_data;
		}
		if (originalFormat == VK_FORMAT_R32G32B32A32_SFLOAT) {
			uint8_t *output_image = new uint8_t[pixel_count * 4];
			auto image_data = static_cast<float *>(data);

#pragma omp parallel for
			for (size_t i = 0; i < pixel_count * 4; i++) {
				// Clamp each channel to the [0, 1] range and then scale to [0, 255]
				output_image[i] = static_cast<uint8_t>(std::fmin(1.0f, std::fmax(0.0f, image_data[i])) * 255);
			}
			delete[] image_data;
			return output_image;
		} else {
			spdlog::error("Image format of the storage image is not supported to be stored correctly!");
			return nullptr;
		}
	}

	void RaytracingRenderer::initProperties(const std::shared_ptr<IProperties> &config,
	const UpdateFlagsHandle &update_flags) {
		if (config->startChild("renderer")) {
			if (config->addUint("recursion_depth", &recursion_depth, 1, 10)) {
				update_flags->setFlag(TARGET_RESET);
			}
			config->endChild();
		}

		for (auto [name, material] : scene_adapter->defaultMaterials) {
			material->initProperties(config, update_flags);
		}
	}

	std::shared_ptr<TextureRepository> RaytracingRenderer::getTextureRepository() {
		return texture_repository;
	}

	std::shared_ptr<MeshRepository> RaytracingRenderer::getMeshRepository() {
		return mesh_repository;
	}

	std::unordered_map<std::string, std::shared_ptr<Material>> RaytracingRenderer::getMaterials() const {
		return scene_adapter->defaultMaterials;
	}
} // namespace RtEngine
