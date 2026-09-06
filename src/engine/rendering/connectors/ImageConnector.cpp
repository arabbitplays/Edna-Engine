#include "ImageConnector.hpp"

#include <utility>

namespace RtEngine
{
    ImageConnector::ImageConnector(std::shared_ptr<ResourceBuilder> resource_builder, VkExtent2D image_extent,
                                   uint32_t image_count, VkFormat format, VkImageUsageFlags usage,
                                   VkImageAspectFlags aspect_flags, DataProvider data_provider)
        : Connector(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
          resource_builder(std::move(resource_builder)), image_extent(image_extent), image_count(image_count),
          format(format), usage(usage), aspect_flags(aspect_flags), data_provider(std::move(data_provider))
    {
        createImages();
    }

    void ImageConnector::write(DescriptorAllocator& allocator, uint32_t binding)
    {
        std::vector<VkImageView> views;
        views.reserve(images.size());
        for (const auto& image : images) {
            views.push_back(image.imageView);
        }
        allocator.writeImages(binding, views, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, getDescriptorType());
    }

    void ImageConnector::createImages()
    {
        images.resize(image_count);
        const VkExtent3D extent_3d{image_extent.width, image_extent.height, 1};

        for (uint32_t i = 0; i < image_count; i++) {
            if (data_provider) {
                std::vector<uint8_t> pixels = data_provider(image_extent);
                images[i] = resource_builder->createImage(pixels.data(), extent_3d, format, VK_IMAGE_TILING_OPTIMAL,
                                                          usage, aspect_flags, VK_IMAGE_LAYOUT_GENERAL);
            } else {
                images[i] = resource_builder->createImage(extent_3d, format, VK_IMAGE_TILING_OPTIMAL, usage,
                                                          aspect_flags);
                resource_builder->transitionImageLayout(
                        images[i].image, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VK_ACCESS_NONE, VK_ACCESS_NONE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            }
        }
    }

    AllocatedImage ImageConnector::getImageAt(uint32_t index) const
    {
        return images[index];
    }

    uint32_t ImageConnector::getImageCount() const
    {
        return image_count;
    }

    VkExtent2D ImageConnector::getExtent() const
    {
        return image_extent;
    }

    void ImageConnector::recreate(VkExtent2D new_extent)
    {
        destroy();
        image_extent = new_extent;
        createImages();
    }

    void ImageConnector::destroy() const
    {
        for (const auto &image : images) {
            resource_builder->destroyImage(image);
        }
    }
} // RtEngine
