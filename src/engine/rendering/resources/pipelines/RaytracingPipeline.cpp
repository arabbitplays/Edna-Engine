#include "../../../../../include/engine/rendering/resources/pipelines/RaytracingPipeline.hpp"

#include <VulkanUtil.hpp>
#include <stdexcept>

namespace RtEngine {
	VkResult createRayTracingPipelinesKhr(VkDevice device, VkDeferredOperationKHR deferred_operation,
										  VkPipelineCache pipeline_cache, uint32_t create_info_count,
										  const VkRayTracingPipelineCreateInfoKHR *p_create_infos,
										  const VkAllocationCallbacks *p_allocator, VkPipeline *p_pipelines) {

		auto func = (PFN_vkCreateRayTracingPipelinesKHR) vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR");
		if (func != nullptr) {
			return func(device, deferred_operation, pipeline_cache, create_info_count, p_create_infos, p_allocator,
						p_pipelines);
		} 			return VK_ERROR_EXTENSION_NOT_PRESENT;
	
	}

	VkResult getRayTracingShaderGroupHandlesKhr(VkDevice device, VkPipeline pipeline, uint32_t first_group,
												uint32_t group_count, size_t data_size, void *p_data) {
		auto func = (PFN_vkGetRayTracingShaderGroupHandlesKHR) vkGetDeviceProcAddr(
				device, "vkGetRayTracingShaderGroupHandlesKHR");
		if (func != nullptr) {
			return func(device, pipeline, first_group, group_count, data_size, p_data);
		} 			return VK_ERROR_EXTENSION_NOT_PRESENT;
	
	}

	// --------------------------------------------------------------------------------------------------------------------------

	void RaytracingPipeline::build() {
		VkDevice device = context->device_manager->getDevice();

		pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
		pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkRayTracingPipelineCreateInfoKHR pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.groupCount = static_cast<uint32_t>(shader_groups.size());
		pipeline_info.pGroups = shader_groups.data();
		pipeline_info.maxPipelineRayRecursionDepth = 31;
		pipeline_info.layout = layout;
		if (createRayTracingPipelinesKhr(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &handle) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create ray tracing pipeline!");
		}

		deletionQueue.pushFunction([&]() {
			vkDestroyPipelineLayout(context->device_manager->getDevice(), layout, nullptr);
			vkDestroyPipeline(context->device_manager->getDevice(), handle, nullptr);
		});
	}

	void RaytracingPipeline::createShaderBindingTables(VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties) {
		VkDevice device = context->device_manager->getDevice();

		std::vector<uint32_t> rgen_indices{0};
		std::vector<uint32_t> miss_indices{1, 2};
		std::vector<uint32_t> hit_indices{3};

		const uint32_t handle_size = raytracing_properties.shaderGroupHandleSize;
		const uint32_t handle_alignment = raytracing_properties.shaderGroupHandleAlignment;
		const uint32_t handle_size_aligned = VulkanUtil::alignedSize(handle_size, handle_alignment);
		const auto group_count = static_cast<uint32_t>(shader_groups.size());
		const uint32_t sbt_size = group_count * handle_size_aligned;
		const uint32_t sbt_usage_flags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		const uint32_t sbt_memory_property_flags =
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		raygenShaderBindingTable =
				context->resource_builder->createBuffer(rgen_indices.size() * handle_size_aligned, sbt_usage_flags, sbt_memory_property_flags);
		missShaderBindingTable =
				context->resource_builder->createBuffer(miss_indices.size() * handle_size_aligned, sbt_usage_flags, sbt_memory_property_flags);
		hitShaderBindingTable =
				context->resource_builder->createBuffer(hit_indices.size() * handle_size_aligned, sbt_usage_flags, sbt_memory_property_flags);
		deletionQueue.pushFunction([&]() {
			context->resource_builder->destroyBuffer(raygenShaderBindingTable);
			context->resource_builder->destroyBuffer(missShaderBindingTable);
			context->resource_builder->destroyBuffer(hitShaderBindingTable);
		});

		std::vector<uint8_t> shader_handle_storage(sbt_size);
		if (getRayTracingShaderGroupHandlesKhr(device, handle, 0, group_count, sbt_size, shader_handle_storage.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to get shader group handles!");
		}

		auto copy_handle = [&](AllocatedBuffer &buffer, std::vector<uint32_t> &indices, uint32_t stride) {
			size_t offset = 0;
			for (unsigned int indice : indices) {
				buffer.update(device, shader_handle_storage.data() + (indice * handle_size_aligned), handle_size,
							  offset);
				offset += stride * sizeof(uint8_t);
			}
		};

		copy_handle(raygenShaderBindingTable, rgen_indices, handle_size_aligned);
		copy_handle(missShaderBindingTable, miss_indices, handle_size_aligned);
		copy_handle(hitShaderBindingTable, hit_indices, handle_size_aligned);
	}

	void RaytracingPipeline::addShaderStage(VkShaderModule shader_module, VkShaderStageFlagBits shader_stage,
								  VkRayTracingShaderGroupTypeKHR shader_group) {
		VkPipelineShaderStageCreateInfo shader_stage_info{};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = shader_stage;
		shader_stage_info.module = shader_module;
		shader_stage_info.pName = "main";
		shader_stages.push_back(shader_stage_info);

		VkRayTracingShaderGroupCreateInfoKHR shader_group_info{};
		shader_group_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shader_group_info.type = shader_group;
		shader_group_info.generalShader = VK_SHADER_UNUSED_KHR;
		shader_group_info.closestHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_info.anyHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_info.intersectionShader = VK_SHADER_UNUSED_KHR;

		if (shader_group == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR) {
			shader_group_info.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		} else if (shader_group == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR) {
			shader_group_info.closestHitShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		}
		shader_groups.push_back(shader_group_info);
	}

	void RaytracingPipeline::setDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> &descriptor_set_layouts) {
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptor_set_layouts.data();
	}

	void RaytracingPipeline::addPushConstant(uint32_t size, VkShaderStageFlags shader_stage) {
		VkPushConstantRange push_constant_range = {};
		push_constant_range.stageFlags = shader_stage;
		push_constant_range.offset = 0;
		push_constant_range.size = size;

		pushConstants.push_back(push_constant_range);
	}

	void RaytracingPipeline::clear() {
		pipelineLayoutInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		shader_stages.clear();
		shader_groups.clear();
		pushConstants.clear();
	}

	void RaytracingPipeline::destroy() { deletionQueue.flush(); }

	VkPipeline RaytracingPipeline::getHandle() const { return handle; }

	VkPipelineLayout RaytracingPipeline::getLayoutHandle() const { return layout; }

	uint32_t RaytracingPipeline::getGroupCount() const { return shader_groups.size(); }

} // namespace RtEngine
