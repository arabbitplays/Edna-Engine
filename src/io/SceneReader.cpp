#include "SceneReader.hpp"

#include <CyclicalCellularAutomaton.hpp>
#include <MeshRenderer.hpp>
#include <Node.hpp>
#include <QuickTimer.hpp>
#include <Rigidbody.hpp>
#include <TransformUtil.hpp>
#include <YAML_glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>
#include "EngineContext.hpp"
#include "Material.hpp"
#include "YamlLoadProperties.hpp"
#include "components/Camera.hpp"

#include "resources/EnvironmentMap.hpp"

namespace RtEngine {
	std::shared_ptr<EngineContext> SceneReader::lockContext() const {
		auto ctx = engine_context.lock();
		if (!ctx) {
			throw std::runtime_error("SceneReader: EngineContext has expired");
		}
		return ctx;
	}

	std::shared_ptr<Scene>
	SceneReader::readScene(const std::string &file_path,
						   std::unordered_map<std::string, std::shared_ptr<Material>> materials) {
		QuickTimer quick_timer("Reading scene from file");

		auto ctx = lockContext();

		try {
			YAML::Node config = YAML::LoadFile(file_path);
			YAML::Node scene_node = config["scene"];

			std::shared_ptr<Material> scene_material = nullptr;
			if (scene_node["material_name"]) {
				auto material_name = scene_node["material_name"].as<std::string>();
				if (!materials.contains(material_name))
					throw std::runtime_error("Material " + material_name + " does not exist");
				scene_material = materials[material_name];
			}

			std::shared_ptr<Scene> scene =
					std::make_shared<Scene>(file_path, scene_material);
			scene->environment_map = std::make_shared<EnvironmentMap>(ctx->texture_repository);

			if (scene_node["lights"]) {
				loadSceneLights(scene_node["lights"], scene);
			}

			if (scene_node["environment_map"]) {
				scene->environment_map->loadFromYaml(scene_node["environment_map"]);
			}

			if (scene_node["meshes"]) {
				for (const auto &mesh_node: scene_node["meshes"]) {
					std::string mesh_path = mesh_node["path"].as<std::string>();
					ctx->mesh_repository->addMesh(mesh_path);
				}
			}

			if (scene_material && scene_node["materials"]) {
				initializeMaterial(scene_node["materials"], scene_material);
			}

			std::shared_ptr<Node> scene_graph_node = std::make_shared<Node>();
			scene_graph_node->name = "root";
			glm::mat4 identity = glm::mat4(1.0f);
			scene_graph_node->transform->setLocalTransform(identity);
			scene_graph_node->children = {};
			for (const auto &yaml_mesh_node: scene_node["nodes"]) {
				scene_graph_node->children.push_back(
						processSceneNodesRecursiv(static_cast<YAML::Node>(yaml_mesh_node), scene));
			}
			scene->addNode(scene_graph_node->name, scene_graph_node);

			return scene;
		} catch (const YAML::Exception &e) {
			throw std::runtime_error(e.what());
		}
	}

	void SceneReader::loadSceneLights(const YAML::Node &lights_node, std::shared_ptr<Scene> &scene) {
		if (lights_node["sun"]) {
			YAML::Node sun_node = lights_node["sun"];
			scene->sun = DirectionalLight(sun_node["direction"].as<glm::vec3>(), sun_node["color"].as<glm::vec3>(),
										  sun_node["intensity"].as<float>());
		}

		uint32_t point_light_index = 0;
		for (auto &point_light_node: lights_node["point_lights"]) {
			scene->pointLights[point_light_index++] =
					PointLight(point_light_node["position"].as<glm::vec3>(), point_light_node["color"].as<glm::vec3>(),
							   point_light_node["intensity"].as<float>());
		}
	}

	void SceneReader::initializeMaterial(const YAML::Node &material_nodes, std::shared_ptr<Material> &material) {
		for (const auto &material_node: material_nodes) {
			material->loadInstance(material_node);
		}
	}

	std::shared_ptr<Node> SceneReader::processSceneNodesRecursiv(const YAML::Node &yaml_node,
																 const std::shared_ptr<Scene> &scene) {
		std::shared_ptr<Node> scene_graph_node = std::make_shared<Node>();
		scene_graph_node->name = yaml_node["name"].as<std::string>();
		readComponents(yaml_node, scene_graph_node);
		scene_graph_node->children = {};
		for (auto &child_node: yaml_node["children"]) {
			scene_graph_node->children.push_back(processSceneNodesRecursiv(child_node, scene));
		}
		scene_graph_node->refreshTransform(glm::mat4(1.0f));

		scene->addNode(scene_graph_node->name, scene_graph_node);
		return scene_graph_node;
	}

	void SceneReader::readComponents(const YAML::Node &yaml_node, std::shared_ptr<Node> &scene_node) {
		auto ctx = lockContext();
		auto update_flags = std::make_shared<UpdateFlags>();
		std::shared_ptr<YamlLoadProperties> properties = std::make_shared<YamlLoadProperties>(yaml_node["components"]);

		for (auto &comp_node: yaml_node["components"]) {
			std::string comp_name = comp_node.first.as<std::string>();
			if (comp_name == Transform::COMPONENT_NAME) {
				scene_node->transform->initProperties(properties, update_flags);
			} else if (comp_name == MeshRenderer::COMPONENT_NAME) {
				std::shared_ptr<MeshRenderer> mesh_component =
						std::make_shared<MeshRenderer>(ctx, scene_node);
				mesh_component->initProperties(properties, update_flags);
				scene_node->addComponent(mesh_component);
			} else if (comp_name == Rigidbody::COMPONENT_NAME) {
				auto rb = std::make_shared<Rigidbody>(scene_node);
				rb->initProperties(properties, update_flags);
				scene_node->addComponent(rb);
			} else if (comp_name == Camera::COMPONENT_NAME) {
				auto cam = std::make_shared<Camera>(ctx, scene_node);
				cam->initProperties(properties, update_flags);
				scene_node->addComponent(cam);
			} else if (comp_name == CyclicalCellularAutomaton::COMPONENT_NAME) {
				auto ca = std::make_shared<CyclicalCellularAutomaton>(ctx, scene_node);
				ca->initProperties(properties, update_flags);
				scene_node->addComponent(ca);
			}
		}
	}
} // namespace RtEngine
