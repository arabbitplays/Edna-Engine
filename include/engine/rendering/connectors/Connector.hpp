#ifndef EDNA_ENGINE_CONNECTOR_HPP
#define EDNA_ENGINE_CONNECTOR_HPP
#include <memory>

#include "DescriptorAllocator.hpp"

namespace RtEngine
{
    class Connector
    {
    public:
        explicit Connector(VkDescriptorType type) : type(type) {}
        virtual ~Connector() = default;

        VkDescriptorType getDescriptorType() const { return type; }

        virtual void write(DescriptorAllocator& allocator, uint32_t binding) = 0;

    private:
        VkDescriptorType type;
    };

    using ConnectorHandle = std::shared_ptr<Connector>;
}

#endif //EDNA_ENGINE_CONNECTOR_HPP
