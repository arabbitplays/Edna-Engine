#include "MeshAssetBuilder.hpp"
#include <iostream>
#include <stdexcept>

#include <AssimpModelLoader.hpp>
#include <ModelLoader.hpp>
#include <cstring>
#include <utility>

namespace RtEngine {
	MeshAsset MeshAssetBuilder::loadMeshAsset(std::string path) {
		AssimpModelLoader loader;
		return loader.loadMeshAsset(resource_path, std::move(path));
	}

	void MeshAssetBuilder::destroyMeshAsset(MeshAsset &mesh_asset) {
		// intentional empty
	}
} // namespace RtEngine
