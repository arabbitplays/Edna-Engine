#ifndef VULKAN_RAYTRACING_MATERIALMANAGER_HPP
#define VULKAN_RAYTRACING_MATERIALMANAGER_HPP

#include <engine/rendering/IScene.hpp>
#include <engine/rendering/resources/materials/MaterialInstance.hpp>
#include <engine/rendering/resources/MaterialTextures.hpp>

namespace RtEngine
{
    class MaterialManager
    {
    public:
        MaterialManager(
            std::shared_ptr<ResourceBuilder> resource_builder, const std::shared_ptr<TextureRepository>& tex_repo);

        void updateMaterialResources(const std::shared_ptr<IScene>& scene);

        AllocatedBuffer createMaterialBuffer(const std::vector<std::shared_ptr<MaterialInstance>>& instances) const;

        void destroy();

    private:
        std::shared_ptr<ResourceBuilder> resource_builder;

        AllocatedBuffer material_buffer;
        std::shared_ptr<MaterialTextures<>> material_textures;
    };
} // namespace RtEngine

#endif // VULKAN_RAYTRACING_MATERIALMANAGER_HPP