#include "ResourceBuilder.hpp"

#include <cstring>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "QuickTimer.hpp"

#include <glm/vector_relational.hpp>
#include <logging/LogManager.hpp>
#include <PathUtil.hpp>
#include <stb_image.h>
#include <stb_image_write.h>

namespace RtEngine
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<ResourceBuilder>();
            return instance;
        }
    } // namespace

    VkDeviceAddress getBufferDeviceAddressKhr(VkDevice device, const VkBufferDeviceAddressInfoKHR* address_info)
    {
        auto func = (PFN_vkGetBufferDeviceAddressKHR)vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR");
        if (func != nullptr)
        {
            return func(device, address_info);
        }
        throw std::runtime_error("Failed to get buffer device address");
    }

    AllocatedBuffer ResourceBuilder::createBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkDevice device = device_manager->getDevice();

        AllocatedBuffer allocated_buffer{};
        allocated_buffer.size = size;

        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_info.flags = 0;

        if (vkCreateBuffer(device, &buffer_info, nullptr, &allocated_buffer.handle) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        VkMemoryRequirements mem_requirements{};
        vkGetBufferMemoryRequirements(device, allocated_buffer.handle, &mem_requirements);

        VkMemoryAllocateFlagsInfo allocate_flags_info{};
        allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, properties);
        alloc_info.pNext = &allocate_flags_info;

        if (vkAllocateMemory(device, &alloc_info, nullptr, &allocated_buffer.bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(device, allocated_buffer.handle, allocated_buffer.bufferMemory, 0);

        VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
        buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        buffer_device_address_info.buffer = allocated_buffer.handle;
        allocated_buffer.deviceAddress = getBufferDeviceAddressKhr(device, &buffer_device_address_info);

        return allocated_buffer;
    }

    AllocatedBuffer ResourceBuilder::stageMemoryToNewBuffer(void* data, size_t size, VkBufferUsageFlags usage)
    {
        VkDevice device = device_manager->getDevice();

        AllocatedBuffer staging_buffer = createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* mapped_data;
        vkMapMemory(device, staging_buffer.bufferMemory, 0, size, 0, &mapped_data);
        memcpy(mapped_data, data, size);
        vkUnmapMemory(device, staging_buffer.bufferMemory);

        AllocatedBuffer mapping_buffer =
            createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copyBuffer(staging_buffer, mapping_buffer, size);
        destroyBuffer(staging_buffer);
        return mapping_buffer;
    }

    uint32_t ResourceBuilder::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties mem_properies;
        vkGetPhysicalDeviceMemoryProperties(device_manager->getPhysicalDevice(), &mem_properies);

        for (uint32_t i = 0; i < mem_properies.memoryTypeCount; i++)
        {
            if (((type_filter & (1 << i)) != 0u) &&
                (mem_properies.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void ResourceBuilder::copyBuffer(AllocatedBuffer src, AllocatedBuffer dst, VkDeviceSize size)
    {
        VkCommandBuffer command_buffer = commandManager->beginSingleTimeCommands();

        VkBufferCopy copy_region{};
        copy_region.size = size;
        vkCmdCopyBuffer(command_buffer, src.handle, dst.handle, 1, &copy_region);

        commandManager->endSingleTimeCommand(command_buffer);
    }

    void ResourceBuilder::destroyBuffer(AllocatedBuffer buffer)
    {
        vkDestroyBuffer(device_manager->getDevice(), buffer.handle, nullptr);
        vkFreeMemory(device_manager->getDevice(), buffer.bufferMemory, nullptr);
    }

    AllocatedImage ResourceBuilder::createImage(VkExtent3D extent, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkImageAspectFlags aspect_flags)
    {
        VkDevice device = device_manager->getDevice();

        AllocatedImage image{};
        image.imageExtent = extent;
        image.imageFormat = format;

        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent = extent;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.format = format;
        image_info.tiling = tiling;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = usage;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(device, &image_info, nullptr, &image.image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements mem_requirements;
        vkGetImageMemoryRequirements(device, image.image, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex =
            findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &alloc_info, nullptr, &image.imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate texture image memory!");
        }

        vkBindImageMemory(device, image.image, image.imageMemory, 0);

        image.imageView = createImageView(image.image, format, aspect_flags);

        return image;
    }

    AllocatedImage ResourceBuilder::createImage(void* data, VkExtent3D extent, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkImageAspectFlags aspect_flags, VkImageLayout target_layout)
    {
        VkDeviceSize image_size = extent.width * extent.height * extent.depth;
        if (format == VK_FORMAT_R8G8B8_SRGB)
        {
            image_size *= 3;
        }
        else if (format == VK_FORMAT_R8G8B8A8_SRGB || format == VK_FORMAT_R8G8B8A8_UNORM)
        {
            image_size *= 4;
        }
        else if (format == VK_FORMAT_R32G32B32A32_UINT)
        {
            image_size *= 16;
        }
        else
        {
            throw std::invalid_argument("Image format not supported!");
        }

        AllocatedImage image =
            createImage(extent, format, tiling, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT, aspect_flags);
        uploadImageData(image, extent, data, image_size, VK_IMAGE_LAYOUT_UNDEFINED, target_layout);
        return image;
    }

    void ResourceBuilder::uploadImageData(AllocatedImage image, VkExtent3D extent, const void* data, VkDeviceSize size,
        VkImageLayout initial_layout, VkImageLayout final_layout)
    {
        VkDevice device = device_manager->getDevice();

        AllocatedBuffer staging_buffer = createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* mapped;
        vkMapMemory(device, staging_buffer.bufferMemory, 0, size, 0, &mapped);
        memcpy(mapped, data, static_cast<size_t>(size));
        vkUnmapMemory(device, staging_buffer.bufferMemory);

        transitionImageLayout(image.image, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, initial_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(staging_buffer.handle, image.image, extent);
        transitionImageLayout(image.image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, final_layout);

        destroyBuffer(staging_buffer);
    }

    Texture ResourceBuilder::loadTextureImage(const std::string& path, TextureType type)
    {
        int texWidth;
        int texHeight;
        int texChannels;
        std::string image_path = resource_path + "/" + path;
        uint8_t* pixels = loadImageData(image_path, &texWidth, &texHeight, &texChannels);

        if (pixels == nullptr)
        {
            throw std::runtime_error("failed to load texture image " + image_path);
        }

        VkFormat format;
        if (type == NORMAL)
        {
            format = VK_FORMAT_R8G8B8A8_UNORM;
        }
        else if (type == PARAMETER)
        {
            format = VK_FORMAT_R8G8B8A8_SRGB;
        }
        else if (type == ENVIRONMENT)
        {
            format = VK_FORMAT_R8G8B8A8_SRGB;
        }
        else
        {
            logger()->error("Texture type not supported!");
        }

        AllocatedImage texture_image = createImage(pixels,
            {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}, format, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        stbi_image_free(pixels);

        return {PathUtil::getFileName(path), type, path, texture_image};
    }

    AllocatedImage ResourceBuilder::loadImage(const std::string& path, VkImageLayout layout)
    {
        int texWidth;
        int texHeight;
        int texChannels;
        std::string image_path = resource_path + "/" + path;
        uint8_t* pixels = loadImageData(image_path, &texWidth, &texHeight, &texChannels);

        if (pixels == nullptr)
        {
            throw std::runtime_error("failed to load texture image " + image_path);
        }

        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        AllocatedImage texture_image =
            createImage(pixels, {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}, format,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT, layout);

        stbi_image_free(pixels);

        return texture_image;
    }

    uint8_t* ResourceBuilder::loadImageData(const std::string& path, int* width, int* height, int* channels)
    {
        return stbi_load(path.c_str(), width, height, channels, STBI_rgb_alpha);
    }

    uint8_t* ResourceBuilder::downloadImage(AllocatedImage image, uint32_t bytes_per_channel)
    {
        QuickTimer timer("download image");

        size_t buffer_size =
            image.imageExtent.width * image.imageExtent.height * image.imageExtent.depth * 4 * bytes_per_channel;
        AllocatedBuffer staging_buffer = createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        transitionImageLayout(image.image, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_NONE, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        copyImageToBuffer(image.image, staging_buffer.handle, image.imageExtent);
        transitionImageLayout(image.image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
            VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_NONE, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

        VkDevice device = device_manager->getDevice();
        void* mapped_data;
        auto* image_data = new uint8_t[buffer_size * bytes_per_channel];
        vkMapMemory(device, staging_buffer.bufferMemory, 0, buffer_size, 0, &mapped_data);
        memcpy(image_data, mapped_data, buffer_size);
        vkUnmapMemory(device, staging_buffer.bufferMemory);

        destroyBuffer(staging_buffer);

        return image_data;
    }

    void ResourceBuilder::transitionImageLayout(VkCommandBuffer command_buffer, VkImage image,
        VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask,
        VkAccessFlags dst_access_mask, VkImageLayout old_layout, VkImageLayout new_layout)
    {

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = src_access_mask;
        barrier.dstAccessMask = dst_access_mask;

        vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void ResourceBuilder::transitionImageLayout(VkImage image, VkPipelineStageFlags src_stage,
        VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
        VkImageLayout old_layout, VkImageLayout new_layout)
    {

        VkCommandBuffer command_buffer = commandManager->beginSingleTimeCommands();
        transitionImageLayout(
            command_buffer, image, src_stage, dst_stage, src_access_mask, dst_access_mask, old_layout, new_layout);
        commandManager->endSingleTimeCommand(command_buffer);
    }

    void ResourceBuilder::copyBufferToImage(VkBuffer buffer, VkImage image, VkExtent3D extent)
    {
        VkCommandBuffer command_buffer = commandManager->beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {.x = 0, .y = 0, .z = 0};
        region.imageExtent = extent;

        vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        commandManager->endSingleTimeCommand(command_buffer);
    }

    void ResourceBuilder::copyImageToBuffer(VkImage image, VkBuffer buffer, VkExtent3D extent)
    {
        VkCommandBuffer command_buffer = commandManager->beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {.x = 0, .y = 0, .z = 0};
        region.imageExtent = extent;

        vkCmdCopyImageToBuffer(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);

        commandManager->endSingleTimeCommand(command_buffer);
    }

    VkImageView ResourceBuilder::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags)
    {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = image;
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = format;

        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask = aspect_flags;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkImageView image_view;
        if (vkCreateImageView(device_manager->getDevice(), &create_info, nullptr, &image_view) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image view!");
        }

        return image_view;
    }

    void ResourceBuilder::destroyImage(AllocatedImage image)
    {
        VkDevice device = device_manager->getDevice();
        vkDestroyImageView(device, image.imageView, nullptr);
        vkDestroyImage(device, image.image, nullptr);
        vkFreeMemory(device, image.imageMemory, nullptr);
    }
} // namespace RtEngine
