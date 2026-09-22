#include "DescriptorLayoutBuilder.hpp"
#include <stdexcept>

namespace RtEngine {
	void DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type, uint32_t descriptor_count) {
		VkDescriptorSetLayoutBinding layout_binding{};
		layout_binding.binding = binding;
		layout_binding.descriptorCount = descriptor_count;
		layout_binding.descriptorType = type;
		layout_binding.pImmutableSamplers = nullptr;

		bindings.push_back(layout_binding);
	}

	VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, uint32_t stage_flags) {
		for (auto &binding: bindings) {
			binding.stageFlags = stage_flags;
		}

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		VkDescriptorSetLayout layout;
		if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		bindings.clear();
		return layout;
	}

	void DescriptorLayoutBuilder::clear() { bindings.clear(); }
} // namespace RtEngine
