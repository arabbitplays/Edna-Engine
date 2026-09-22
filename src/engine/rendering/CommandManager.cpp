#include "CommandManager.hpp"
#include <stdexcept>

namespace RtEngine {
	CommandManager::CommandManager() = default;

	CommandManager::CommandManager(const std::shared_ptr<DeviceManager> &device_manager) : deviceManager(device_manager) {
		createCommandPool();
	}

	void CommandManager::createCommandPool() {
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = deviceManager->getQueueIndices().graphicsAndComputeFamily.value();

		if (vkCreateCommandPool(deviceManager->getDevice(), &pool_info, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	VkCommandBuffer CommandManager::beginSingleTimeCommands() const {
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = commandPool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(deviceManager->getDevice(), &alloc_info, &command_buffer);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &begin_info);

		return command_buffer;
	}

	void CommandManager::endSingleTimeCommand(const VkCommandBuffer command_buffer) const {
		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		VkQueue graphics_queue = deviceManager->getQueue(GRAPHICS);
		vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_queue);

		vkFreeCommandBuffers(deviceManager->getDevice(), commandPool, 1, &command_buffer);
	}

	std::vector<VkCommandBuffer> CommandManager::allocatePrimaryCommandBuffers(uint32_t count) const {
		std::vector<VkCommandBuffer> buffers(count);

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = commandPool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = count;

		if (vkAllocateCommandBuffers(deviceManager->getDevice(), &alloc_info, buffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
		return buffers;
	}

	void CommandManager::destroy() const {
		vkDestroyCommandPool(deviceManager->getDevice(), commandPool, nullptr);
	}
} // namespace RtEngine
