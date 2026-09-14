#ifndef EDNA_ENGINE_CONNECTORLAYOUT_HPP
#define EDNA_ENGINE_CONNECTORLAYOUT_HPP
#include <vector>

#include "Connector.hpp"
#include "DescriptorAllocator.hpp"
#include "DeviceManager.hpp"

namespace RtEngine
{
    class ConnectorLayout
    {
    public:
        ConnectorLayout(std::shared_ptr<DeviceManager> device_manager,
                        std::shared_ptr<DescriptorAllocator> descriptor_allocator);
        ~ConnectorLayout() = default;

        void addConnector(uint32_t binding, ConnectorHandle connector);

        VkDescriptorSetLayout createLayout(VkShaderStageFlags stage_flags);
        VkDescriptorSet allocateSet(VkDescriptorSetLayout layout);
        void writeInto(VkDescriptorSet set);

        std::vector<ConnectorHandle> getConnectors();
    private:
        std::shared_ptr<DeviceManager> device_manager;
        std::shared_ptr<DescriptorAllocator> descriptor_allocator;

        std::vector<ConnectorHandle> connectors{};
    };
} // RtEnginge

#endif //EDNA_ENGINE_CONNECTORLAYOUT_HPP
