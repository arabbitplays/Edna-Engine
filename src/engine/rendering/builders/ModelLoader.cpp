#include "ModelLoader.hpp"

#include <iostream>
#include <PathUtil.hpp>
#include <stdexcept>

namespace RtEngine
{
    MeshAsset ModelLoader::loadMeshAsset(const std::string& resources_path, const std::string& path)
    {
        MeshBuffers mesh_buffers{};

        std::string full_path = resources_path + "/" + path;
        loadData(full_path, mesh_buffers.vertices, mesh_buffers.indices);

        MeshAsset mesh_asset{};
        mesh_asset.name = PathUtil::getFileName(path);
        mesh_asset.path = path;
        mesh_asset.meshBuffers = mesh_buffers;
        mesh_asset.vertex_count = mesh_buffers.indices.size();
        mesh_asset.triangle_count = mesh_buffers.indices.size() / 3;

        mesh_asset.instance_data = {};
        return mesh_asset;
    }
} // namespace RtEngine
