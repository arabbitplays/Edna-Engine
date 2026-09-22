#ifndef EDNA_ENGINE_BUFFERCONNECTORFACTORY_HPP
#define EDNA_ENGINE_BUFFERCONNECTORFACTORY_HPP
#include "BufferConnector.hpp"
#include "DeviceManager.hpp"
#include "ResourceBuilder.hpp"

#include <memory>

namespace RtEngine
{
    class BufferConnectorFactory
    {
    public:
        static std::shared_ptr<BufferConnector> createStorageBuffer(
            const std::shared_ptr<ResourceBuilder>& resource_builder,
            const std::shared_ptr<DeviceManager>& device_manager, VkDeviceSize buffer_size, uint32_t buffer_count);

        static std::shared_ptr<BufferConnector> createUniformBuffer(
            const std::shared_ptr<ResourceBuilder>& resource_builder,
            const std::shared_ptr<DeviceManager>& device_manager, VkDeviceSize buffer_size, uint32_t buffer_count);
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_BUFFERCONNECTORFACTORY_HPP
