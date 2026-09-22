#ifndef VULKAN_RAYTRACING_ISERIALIZABLE_HPP
#define VULKAN_RAYTRACING_ISERIALIZABLE_HPP
#include "IProperties.hpp"
#include "UpdateFlagValue.hpp"

#include <memory>

namespace RtEngine
{
    class ISerializable;
    typedef std::shared_ptr<ISerializable> SerializableHandle;

    class ISerializable
    {
    public:
        virtual ~ISerializable() = default;

        virtual void initProperties(
            const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& update_flags) = 0;
    };
} // namespace RtEngine

#endif // VULKAN_RAYTRACING_ISERIALIZABLE_HPP