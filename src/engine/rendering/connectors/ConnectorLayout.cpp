#include "ConnectorLayout.hpp"

#include <cassert>
#include <utility>

#include "DescriptorLayoutBuilder.hpp"
#include "spdlog/spdlog.h"

namespace RtEngine
{
    ConnectorLayout::ConnectorLayout(std::shared_ptr<DeviceManager> device_manager,
                                     std::shared_ptr<DescriptorAllocator> descriptor_allocator)
        : device_manager(std::move(device_manager)), descriptor_allocator(std::move(descriptor_allocator))
    {
    }

    void ConnectorLayout::addConnector(uint32_t binding, ConnectorHandle connector)
    {
        if (connectors.size() <= binding)
        {
            connectors.resize(binding + 1);
        }

        if (connectors[binding] != nullptr)
        {
            SPDLOG_WARN("Overwriting connector at binding " + std::to_string(binding));
        }

        connectors[binding] = std::move(connector);
    }

    VkDescriptorSetLayout ConnectorLayout::createLayout(VkShaderStageFlags stage_flags)
    {
        DescriptorLayoutBuilder builder;
        for (uint32_t binding = 0; binding < connectors.size(); binding++)
        {
            assert(connectors[binding] != nullptr && "ConnectorLayout has nullptr connector at binding");
            builder.addBinding(binding, connectors[binding]->getDescriptorType());
        }
        return builder.build(device_manager->getDevice(), stage_flags);
    }

    VkDescriptorSet ConnectorLayout::writeConnectors(VkDescriptorSetLayout layout)
    {
        for (uint32_t binding = 0; binding < connectors.size(); binding++)
        {
            if (connectors[binding] == nullptr)
            {
                continue;
            }
            connectors[binding]->write(*descriptor_allocator, binding);
        }

        VkDescriptorSet set = descriptor_allocator->allocate(device_manager->getDevice(), layout);
        descriptor_allocator->updateSet(device_manager->getDevice(), set);
        descriptor_allocator->clearWrites();
        return set;
    }

    std::vector<ConnectorHandle> ConnectorLayout::getConnectors()
    {
        return connectors;
    }
} // RtEnginge
