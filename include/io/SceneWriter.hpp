#ifndef SCENEWRITER_H
#define SCENEWRITER_H

#include <../engine/scene_graph/Scene.hpp>
#include <string>
#include <yaml-cpp/yaml.h>

namespace RtEngine {
	class SceneWriter {
	public:
		SceneWriter() = default;

		void writeScene(const std::string &filename, const std::shared_ptr<Scene>& scene);

	private:
		static void writeMaterial(YAML::Emitter &out, const std::shared_ptr<Material> &material);
		static void writeSceneLights(YAML::Emitter &out, const std::shared_ptr<Scene> &scene);

		static YAML::Node writeComponents(const std::shared_ptr<Node> &node);
		void writeSceneNode(YAML::Emitter &out, const std::shared_ptr<Node> &node);
	};

} // namespace RtEngine
#endif // SCENEWRITER_H
