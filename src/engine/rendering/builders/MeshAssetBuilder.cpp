#include "MeshAssetBuilder.hpp"

#include <AssimpModelLoader.hpp>
#include <cstring>
#include <iostream>
#include <ModelLoader.hpp>
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
