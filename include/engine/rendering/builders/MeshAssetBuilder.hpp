#ifndef BASICS_MESHASSETBUILDER_HPP
#define BASICS_MESHASSETBUILDER_HPP

#include <engine/rendering/builders/ResourceBuilder.hpp>

#include <engine/rendering/resources/AccelerationStructure.hpp>
#include <engine/rendering/resources/MeshAsset.hpp>
#include <engine/rendering/builders/ModelLoader.hpp>

namespace RtEngine
{
    class MeshAssetBuilder
    {
    public:
        MeshAssetBuilder() = default;
        MeshAssetBuilder(VkDevice device, const std::string& resource_path)
            : device(device), resource_path(resource_path) {};
        MeshAsset loadMeshAsset(std::string path);
        void destroyMeshAsset(MeshAsset& meshAsset);

    private:
        VkDevice device;
        std::string resource_path;
    };

} // namespace RtEngine
#endif // BASICS_MESHASSETBUILDER_HPP
