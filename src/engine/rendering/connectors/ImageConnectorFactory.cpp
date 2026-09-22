#include "ImageConnectorFactory.hpp"

#include "RandomUtil.hpp"

namespace RtEngine
{
    namespace
    {
        std::vector<uint8_t> generateRngPixels(VkExtent2D extent)
        {
            const size_t uint_count = static_cast<size_t>(extent.width) * extent.height * 4;
            std::vector<uint8_t> pixels(uint_count * sizeof(uint32_t));
            auto* ints = reinterpret_cast<uint32_t*>(pixels.data());
            for (size_t i = 0; i < uint_count; i++)
            {
                ints[i] = RandomUtil::generateInt();
            }
            return pixels;
        }
    } // namespace

    std::shared_ptr<ImageConnector> ImageConnectorFactory::createRenderTargetConnector(
        const std::shared_ptr<ResourceBuilder>& resource_builder, VkExtent2D extent, uint32_t image_count)
    {
        return std::make_shared<ImageConnector>(resource_builder, extent, image_count, VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    std::shared_ptr<ImageConnector> ImageConnectorFactory::createRngTextureConnector(
        const std::shared_ptr<ResourceBuilder>& resource_builder, VkExtent2D extent, uint32_t image_count)
    {
        return std::make_shared<ImageConnector>(resource_builder, extent, image_count, VK_FORMAT_R32G32B32A32_UINT,
            VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT, &generateRngPixels);
    }
} // namespace RtEngine
