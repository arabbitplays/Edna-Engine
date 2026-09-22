#ifndef SCENEREADER_H
#define SCENEREADER_H

#include <../engine/scene_graph/Scene.hpp>
#include <string>
#include <yaml-cpp/yaml.h>

namespace RtEngine
{
    class SceneReader
    {
    public:
        SceneReader() = default;
        SceneReader(const std::shared_ptr<EngineContext>& engine_context) : engine_context(engine_context)
        {
        }

        std::shared_ptr<Scene> readScene(
            const std::string& file_path, std::unordered_map<std::string, std::shared_ptr<Material>> materials);
        static void loadSceneLights(const YAML::Node& lights_node, std::shared_ptr<Scene>& scene);
        static void initializeMaterial(const YAML::Node& material_node, std::shared_ptr<Material>& material);

    private:
        std::shared_ptr<Node> processSceneNodesRecursiv(
            const YAML::Node& yaml_node, const std::shared_ptr<Scene>& scene);
        void readComponents(const YAML::Node& yaml_node, std::shared_ptr<Node>& scene_node);

        std::shared_ptr<EngineContext> lockContext() const;

        // weak_ptr to break the EngineContext -> SceneManager -> SceneReader -> EngineContext cycle.
        std::weak_ptr<EngineContext> engine_context;
    };

} // namespace RtEngine
#endif // SCENEREADER_H
