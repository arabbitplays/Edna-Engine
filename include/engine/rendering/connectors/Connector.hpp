#ifndef EDNA_ENGINE_CONNECTOR_HPP
#define EDNA_ENGINE_CONNECTOR_HPP
#include "DescriptorAllocator.hpp"

#include <memory>

namespace RtEngine
{
    class Connector
    {
    public:
        explicit Connector(VkDescriptorType type) : type(type)
        {
        }
        virtual ~Connector() = default;

        VkDescriptorType getDescriptorType() const
        {
            return type;
        }
        virtual uint32_t getDescriptorCount() const
        {
            return 1;
        }

        virtual void write(DescriptorAllocator& allocator, uint32_t binding) = 0;

    private:
        VkDescriptorType type;
    };

    using ConnectorHandle = std::shared_ptr<Connector>;
} // namespace RtEngine

#endif // EDNA_ENGINE_CONNECTOR_HPP
