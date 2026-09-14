#ifndef EDNA_ENGINE_BUFFERCONNECTOR_HPP
#define EDNA_ENGINE_BUFFERCONNECTOR_HPP

#include "Connector.hpp"
#include "DeviceManager.hpp"
#include "ResourceBuilder.hpp"

namespace RtEngine
{
    class BufferConnector : public Connector
    {
    public:
        BufferConnector(std::shared_ptr<ResourceBuilder> resource_builder,
                        std::shared_ptr<DeviceManager> device_manager,
                        VkDescriptorType type, VkDeviceSize buffer_size, uint32_t buffer_count,
                        VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
        ~BufferConnector() override = default;

        void uploadData(uint32_t index, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

        AllocatedBuffer getBufferAt(uint32_t index) const;
        uint32_t getBufferCount() const;
        VkDeviceSize getBufferSize() const;

        uint32_t getDescriptorCount() const override { return buffer_count; }

        void destroy();

        void write(DescriptorAllocator& allocator, uint32_t binding) override;

    private:
        void createBuffers();

        std::shared_ptr<ResourceBuilder> resource_builder;
        std::shared_ptr<DeviceManager> device_manager;

        VkDeviceSize buffer_size;
        uint32_t buffer_count;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags memory_properties;

        std::vector<AllocatedBuffer> buffers;
    };
} // RtEngine

#endif //EDNA_ENGINE_BUFFERCONNECTOR_HPP
