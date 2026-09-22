#include "MetalRoughMaterial.hpp"

#include <DescriptorLayoutBuilder.hpp>
#include <OptionsWindow.hpp>
#include <VulkanUtil.hpp>
#include <metal_rough_closesthit.rchit.spv.h>
#include <metal_rough_miss.rmiss.spv.h>
#include <metal_rough_raygen.rgen.spv.h>
#include <shadow_miss.rmiss.spv.h>

#include <format>
#include <logging/LogManager.hpp>

#include "MetalRoughInstance.hpp"

namespace RtEngine {
	namespace {
		Logging::LoggerHandle& logger() {
			static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<MetalRoughMaterial>();
			return instance;
		}
	}

	void MetalRoughMaterial::buildPipelines(VkDescriptorSetLayout scene_layout) {
		DescriptorLayoutBuilder layout_builder;
		pipeline = std::make_shared<RaytracingPipeline>(vulkan_context);
		VkDevice device = vulkan_context->device_manager->getDevice();

		layout_builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layout_builder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64); // TODO make this dynamic depending on the scene

		materialLayout = layout_builder.build(device, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
		mainDeletionQueue.pushFunction([&]() {
			vkDestroyDescriptorSetLayout(vulkan_context->device_manager->getDevice(), materialLayout, nullptr);
		});
		materialDescriptorSet = descriptorAllocator.allocate(vulkan_context->device_manager->getDevice(), materialLayout);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{scene_layout, materialLayout};
		pipeline->setDescriptorSetLayouts(descriptor_set_layouts);

		pipeline->addPushConstant(16 * sizeof(uint32_t), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
																  VK_SHADER_STAGE_RAYGEN_BIT_KHR |
																  VK_SHADER_STAGE_MISS_BIT_KHR);

		VkShaderModule raygen_shader_module = VulkanUtil::createShaderModule(
				device, oschd_metal_rough_raygen_rgen_spv_size(), oschd_metal_rough_raygen_rgen_spv());
		VkShaderModule miss_shader_module = VulkanUtil::createShaderModule(
				device, oschd_metal_rough_miss_rmiss_spv_size(), oschd_metal_rough_miss_rmiss_spv());
		VkShaderModule shadow_miss_shader_module = VulkanUtil::createShaderModule(
				device, oschd_shadow_miss_rmiss_spv_size(), oschd_shadow_miss_rmiss_spv());
		VkShaderModule closest_hit_shader_module = VulkanUtil::createShaderModule(
				device, oschd_metal_rough_closesthit_rchit_spv_size(), oschd_metal_rough_closesthit_rchit_spv());

		pipeline->addShaderStage(raygen_shader_module, VK_SHADER_STAGE_RAYGEN_BIT_KHR,
								 VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR);
		pipeline->addShaderStage(miss_shader_module, VK_SHADER_STAGE_MISS_BIT_KHR,
								 VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR);
		pipeline->addShaderStage(shadow_miss_shader_module, VK_SHADER_STAGE_MISS_BIT_KHR,
								 VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR);
		pipeline->addShaderStage(closest_hit_shader_module, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
								 VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR);

		pipeline->build();

		mainDeletionQueue.pushFunction([&]() { pipeline->destroy(); });

		vkDestroyShaderModule(device, raygen_shader_module, nullptr);
		vkDestroyShaderModule(device, miss_shader_module, nullptr);
		vkDestroyShaderModule(device, shadow_miss_shader_module, nullptr);
		vkDestroyShaderModule(device, closest_hit_shader_module, nullptr);
	}

	void MetalRoughMaterial::writeMaterial(AllocatedBuffer material_buffer, std::shared_ptr<MaterialTextures<>> material_textures) {
		descriptorAllocator.writeBuffer(0, material_buffer.handle, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		descriptorAllocator.writeImages(1, material_textures->getOrderedImageViews(), sampler, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
										VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		VkDevice device = vulkan_context->device_manager->getDevice();
		descriptorAllocator.updateSet(device, materialDescriptorSet);
		descriptorAllocator.clearWrites();
	}

	std::shared_ptr<MaterialInstance> MetalRoughMaterial::loadInstance(const YAML::Node& yaml_node) {
		std::shared_ptr<MaterialInstance> instance = std::make_shared<MetalRoughInstance>("", tex_repo);
		instance->loadResources(yaml_node);
		if (instances.contains(instance->name)) {
			logger()->warn(std::format("Material instance with name {} already exists!", instance->name));
			return instances[instance->name];
		}

		instances[instance->name] = instance;
		return instance;
	}

	void MetalRoughMaterial::initProperties(const std::shared_ptr<IProperties> &config, const UpdateFlagsHandle &update_flags) {
		bool reset_required = false;
		if (config->startChild(name)) {
			reset_required |= config->addBool("normal_mapping", &normal_mapping);
			reset_required |= config->addBool("nearest_neighbor_estimation", &sample_lights);
			reset_required |= config->addBool("bsdf_importance_sampling", &sample_bsdf);
			reset_required |= config->addBool("russian_roulette", &russian_roulette);
			config->endChild();
		}

		if (reset_required) {
			update_flags->setFlag(TARGET_RESET);
		}
	}

	void MetalRoughMaterial::getPushConstantValues(std::vector<int32_t> &push_constants) {
		push_constants.push_back(static_cast<int32_t>(normal_mapping));
		push_constants.push_back(static_cast<int32_t>(sample_lights));
		push_constants.push_back(static_cast<int32_t>(sample_bsdf));
		push_constants.push_back(static_cast<int32_t>(russian_roulette));
	}

	void MetalRoughMaterial::reset() {
		instances.clear();
		Material::reset();
	}
} // namespace RtEngine
