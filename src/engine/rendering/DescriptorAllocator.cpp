#include "DescriptorAllocator.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace RtEngine {
	VkDescriptorPool DescriptorAllocator::getPool(VkDevice device) {
		VkDescriptorPool new_pool;
		if (!readyPools.empty()) {
			new_pool = readyPools.back();
			readyPools.pop_back();
		} else {
			new_pool = createPool(device, setsPerPool, ratios);

			setsPerPool = setsPerPool * GROW_RATIO;
			setsPerPool = std::min(setsPerPool, MAX_SET_COUNT);
		}

		return new_pool;
	}

	VkDescriptorPool DescriptorAllocator::createPool(const VkDevice device, const uint32_t set_count,
													 std::span<DescriptorAllocator::PoolSizeRatio> pool_ratios) {
		std::vector<VkDescriptorPoolSize> pool_sizes;
		for (auto [type, ratio]: pool_ratios) {
			VkDescriptorPoolSize pool_size{.type = type, .descriptorCount = static_cast<uint32_t>(ratio * static_cast<float>(set_count))};
			pool_sizes.push_back(pool_size);
		}

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = set_count;

		VkDescriptorPool descriptor_pool;
		if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
		return descriptor_pool;
	}

	VkDescriptorPool DescriptorAllocator::createPool(const VkDevice device, const std::vector<VkDescriptorPoolSize>& pool_sizes,
													 const VkDescriptorPoolCreateFlags flags) {
		VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = flags;
		pool_info.maxSets = 1;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		};

		return descriptor_pool;
	}

	void DescriptorAllocator::init(VkDevice device, uint32_t initial_set_count, std::span<PoolSizeRatio> pool_ratios) {
		ratios.clear();

		for (auto r: pool_ratios) {
			ratios.push_back(r);
		}

		VkDescriptorPool new_pool = createPool(device, initial_set_count, pool_ratios);
		setsPerPool = initial_set_count * GROW_RATIO;

		readyPools.push_back(new_pool);
	}

	void DescriptorAllocator::clearPools(VkDevice device) {
		for (auto *p: readyPools) {
			vkResetDescriptorPool(device, p, 0);
		}

		for (auto *p: fullPools) {
			vkResetDescriptorPool(device, p, 0);
			readyPools.push_back(p);
		}
		fullPools.clear();
	}

	void DescriptorAllocator::destroyPools(VkDevice device) {
		for (auto *p: readyPools) {
			vkDestroyDescriptorPool(device, p, nullptr);
		}
		readyPools.clear();

		for (auto *p: fullPools) {
			vkDestroyDescriptorPool(device, p, nullptr);
		}
		fullPools.clear();
	}

	VkDescriptorSet DescriptorAllocator::allocate(const VkDevice device, const VkDescriptorSetLayout layout, const void *p_next) {
		VkDescriptorPool pool_to_use = getPool(device);

		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.pNext = p_next;
		alloc_info.descriptorPool = pool_to_use;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &layout;

		VkDescriptorSet descriptor_set;
		VkResult result = vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set);

		if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
			fullPools.push_back(pool_to_use);

			pool_to_use = getPool(device);
			alloc_info.descriptorPool = pool_to_use;

			if (vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor set!");
			}
		}

		readyPools.push_back(pool_to_use);
		return descriptor_set;
	}

	void DescriptorAllocator::writeBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize size, uint32_t offset,
										  VkDescriptorType type) {
		writeBuffers(binding, {buffer}, size, offset, type);
	}

	void DescriptorAllocator::writeBuffer(uint32_t binding, VkBuffer buffer, uint32_t offset, VkDescriptorType type) {
		writeBuffer(binding, buffer, VK_WHOLE_SIZE, offset, type);
	}

	void DescriptorAllocator::writeBuffers(const uint32_t binding, const std::vector<VkBuffer>& buffers,
										   const VkDeviceSize size, const uint32_t offset, const VkDescriptorType type) {
		BufferInfoWrapper wrapper{};
		wrapper.buffer_infos.resize(buffers.size());
		for (size_t i = 0; i < buffers.size(); i++) {
			wrapper.buffer_infos[i] = VkDescriptorBufferInfo{.buffer = buffers[i], .offset = offset, .range = size};
		}
		bufferInfos.push_back(wrapper);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorCount = static_cast<uint32_t>(buffers.size());
		write.descriptorType = type;
		write.pBufferInfo = bufferInfos.back().buffer_infos.data();

		writes.push_back(write);
	}

	void DescriptorAllocator::writeImage(uint32_t binding, VkImageView image_view, VkSampler sampler,
										 VkImageLayout layout, VkDescriptorType type) {
		ImageInfoWrapper wrapper{};
		wrapper.image_infos.push_back(
			VkDescriptorImageInfo{.sampler = sampler, .imageView = image_view, .imageLayout = layout});
		imageInfos.push_back(wrapper);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pImageInfo = imageInfos.back().image_infos.data();

		writes.push_back(write);
	}

	void DescriptorAllocator::writeImages(const uint32_t binding, const std::vector<VkImageView>& image_views, const VkSampler sampler,
										  const VkImageLayout layout, const VkDescriptorType type) {
		ImageInfoWrapper wrapper{};
		wrapper.image_infos.resize(image_views.size());
		for (size_t i = 0; i < image_views.size(); i++) {
			wrapper.image_infos[i] = VkDescriptorImageInfo{
					.sampler = sampler, .imageView = image_views[i], .imageLayout = layout};
		}
		imageInfos.push_back(wrapper);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorCount = static_cast<uint32_t>(image_views.size());
		write.descriptorType = type;
		write.pImageInfo = imageInfos.back().image_infos.data();

		writes.push_back(write);
	}

	void DescriptorAllocator::writeAccelerationStructure(uint32_t binding,
														 VkAccelerationStructureKHR acceleration_structure,
														 VkDescriptorType type) {
		auto& wrapper = accelerationStructureInfos.emplace_back();

		wrapper.structure = acceleration_structure;

		wrapper.info = VkWriteDescriptorSetAccelerationStructureKHR{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
			.pNext = nullptr,
			.accelerationStructureCount = 1,
			.pAccelerationStructures = &wrapper.structure,
		};

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = binding;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pNext = &wrapper.info;

		writes.push_back(write);
	}

	void DescriptorAllocator::updateSet(const VkDevice &device, const VkDescriptorSet &set) {
		for (auto &write: writes) {
			write.dstSet = set;
		}

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, VK_NULL_HANDLE);
	}

	void DescriptorAllocator::clearWrites() {
		bufferInfos.clear();
		imageInfos.clear();
		accelerationStructureInfos.clear();
		writes.clear();
	}
} // namespace RtEngine
