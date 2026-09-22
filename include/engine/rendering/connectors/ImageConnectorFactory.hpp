#ifndef EDNA_ENGINE_IMAGECONNECTORFACTORY_HPP
#define EDNA_ENGINE_IMAGECONNECTORFACTORY_HPP
#include "ImageConnector.hpp"
#include "ResourceBuilder.hpp"

#include <memory>

namespace RtEngine
{
    class ImageConnectorFactory
    {
    public:
        static std::shared_ptr<ImageConnector> createRenderTargetConnector(
            const std::shared_ptr<ResourceBuilder>& resource_builder, VkExtent2D extent, uint32_t image_count);

        static std::shared_ptr<ImageConnector> createRngTextureConnector(
            const std::shared_ptr<ResourceBuilder>& resource_builder, VkExtent2D extent, uint32_t image_count);
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_IMAGECONNECTORFACTORY_HPP
