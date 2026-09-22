//
// Created by oschdi on 17.01.26.
//

#include "EnvironmentMap.hpp"

#include "DescriptorAllocator.hpp"

#include <utility>

namespace RtEngine
{
    EnvironmentMap::EnvironmentMap(std::shared_ptr<TextureRepository> tex_repo)
        : tex_repo(std::move(std::move(tex_repo)))
    {
    }

    void EnvironmentMap::writeToDescriptor(
        const std::shared_ptr<DescriptorAllocator>& descriptor_allocator, const VkSampler sampler)
    {
        std::vector<VkImageView> views{};
        views.reserve(textures.size());
        for (auto& texture : textures)
        {
            views.push_back(texture->image.imageView);
        }

        for (uint32_t i = views.size(); i < 6; i++)
        {
            views.push_back(tex_repo->getDefaultTex(ENVIRONMENT)->image.imageView);
        }

        descriptor_allocator->writeImages(
            8, views, sampler, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }

    void EnvironmentMap::loadFromYaml(YAML::Node node)
    {
        for (const auto& texture_node : node["textures"])
        {
            auto path = texture_node.as<std::string>();
            textures.push_back(tex_repo->addTexture(path, ENVIRONMENT));
        }
    }
} // namespace RtEngine