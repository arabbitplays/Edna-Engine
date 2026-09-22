#ifndef EDNA_ENGINE_IMAGECONNECTOR_HPP
#define EDNA_ENGINE_IMAGECONNECTOR_HPP
#include "Connector.hpp"
#include "ResourceBuilder.hpp"

#include <functional>

namespace RtEngine
{
    class ImageConnector : public Connector
    {
    public:
        using DataProvider = std::function<std::vector<uint8_t>(VkExtent2D)>;

        ImageConnector(std::shared_ptr<ResourceBuilder> resource_builder, VkExtent2D image_extent, uint32_t image_count,
            VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect_flags,
            DataProvider data_provider = nullptr);

        AllocatedImage getImageAt(uint32_t index) const;
        uint32_t getImageCount() const;
        VkExtent2D getExtent() const;

        void recreate(VkExtent2D new_extent);
        void destroy();

        void write(DescriptorAllocator& allocator, uint32_t binding) override;

    private:
        void createImages();

        std::shared_ptr<ResourceBuilder> resource_builder;

        VkExtent2D image_extent;
        uint32_t image_count;
        VkFormat format;
        VkImageUsageFlags usage;
        VkImageAspectFlags aspect_flags;
        DataProvider data_provider;

        std::vector<AllocatedImage> images;
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_IMAGECONNECTOR_HPP
