#include <engine/rendering/builders/MeshAssetBuilder.hpp>

#include <engine/rendering/builders/AssimpModelLoader.hpp>
#include <cstring>
#include <iostream>
#include <engine/rendering/builders/ModelLoader.hpp>
#include <stdexcept>
#include <utility>

namespace RtEngine
{
    MeshAsset MeshAssetBuilder::loadMeshAsset(std::string path)
    {
        AssimpModelLoader loader;
        return loader.loadMeshAsset(resource_path, std::move(path));
    }

    void MeshAssetBuilder::destroyMeshAsset(MeshAsset& mesh_asset)
    {
        // intentional empty
    }
} // namespace RtEngine
