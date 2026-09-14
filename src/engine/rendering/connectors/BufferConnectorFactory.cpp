#include "BufferConnectorFactory.hpp"

namespace RtEngine {
    std::shared_ptr<BufferConnector> BufferConnectorFactory::createStorageBuffer(
        const std::shared_ptr<ResourceBuilder>& resource_builder,
        const std::shared_ptr<DeviceManager>& device_manager,
        VkDeviceSize buffer_size,
        uint32_t buffer_count) {
        return std::make_shared<BufferConnector>(
            resource_builder, device_manager,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_size, buffer_count,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    std::shared_ptr<BufferConnector> BufferConnectorFactory::createUniformBuffer(
        const std::shared_ptr<ResourceBuilder>& resource_builder,
        const std::shared_ptr<DeviceManager>& device_manager,
        VkDeviceSize buffer_size,
        uint32_t buffer_count) {
        return std::make_shared<BufferConnector>(
            resource_builder, device_manager,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, buffer_size, buffer_count,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
} // RtEngine
