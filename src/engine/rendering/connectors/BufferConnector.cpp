#include "BufferConnector.hpp"

#include <cassert>
#include <utility>

namespace RtEngine
{
    BufferConnector::BufferConnector(std::shared_ptr<ResourceBuilder> resource_builder,
                                     std::shared_ptr<DeviceManager> device_manager,
                                     VkDescriptorType type, VkDeviceSize buffer_size, uint32_t buffer_count,
                                     VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties)
        : Connector(type),
          resource_builder(std::move(resource_builder)), device_manager(std::move(device_manager)),
          buffer_size(buffer_size), buffer_count(buffer_count), usage(usage), memory_properties(memory_properties)
    {
        assert((type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
               && "BufferConnector only supports uniform and storage buffer descriptor types");
        assert(buffer_count > 0 && "BufferConnector needs at least one buffer");
        assert(buffer_size > 0 && "BufferConnector needs a non-zero buffer size");

        createBuffers();
    }

    void BufferConnector::createBuffers()
    {
        buffers.resize(buffer_count);
        for (uint32_t i = 0; i < buffer_count; i++) {
            buffers[i] = resource_builder->createBuffer(buffer_size, usage, memory_properties);
        }
    }

    void BufferConnector::uploadData(uint32_t index, const void* data, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(index < buffers.size() && "BufferConnector upload index out of range");
        assert(offset + size <= buffer_size && "BufferConnector upload exceeds buffer size");
        assert((memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
               && "BufferConnector::uploadData requires host-visible memory");

        buffers[index].update(device_manager->getDevice(), const_cast<void*>(data), size, offset);
    }

    void BufferConnector::write(DescriptorAllocator& allocator, uint32_t binding)
    {
        std::vector<VkBuffer> handles;
        handles.reserve(buffers.size());
        for (const auto& buffer : buffers) {
            handles.push_back(buffer.handle);
        }
        allocator.writeBuffers(binding, handles, buffer_size, 0, getDescriptorType());
    }

    AllocatedBuffer BufferConnector::getBufferAt(uint32_t index) const
    {
        return buffers[index];
    }

    uint32_t BufferConnector::getBufferCount() const
    {
        return buffer_count;
    }

    VkDeviceSize BufferConnector::getBufferSize() const
    {
        return buffer_size;
    }

    void BufferConnector::destroy()
    {
        for (const auto& buffer : buffers) {
            resource_builder->destroyBuffer(buffer);
        }
        buffers.clear();
    }
} // RtEngine
