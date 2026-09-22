#ifndef MODELLOADER_HPP
#define MODELLOADER_HPP

#include <../rendering/resources/Vertex.hpp>
#include <MeshAsset.hpp>
#include <string.h>
#include <vector>

namespace RtEngine
{
    class ModelLoader
    {
    public:
        ModelLoader() = default;
        virtual ~ModelLoader() = default;

        MeshAsset loadMeshAsset(const std::string& ressources_path, const std::string& path);

    protected:
        virtual void loadData(std::string path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) = 0;
    };

} // namespace RtEngine
#endif // MODELLOADER_HPP
