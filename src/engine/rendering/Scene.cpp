#include "Scene.hpp"

#include "SceneUtil.hpp"

#include <components/Camera.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <PhongMaterial.hpp>

namespace RtEngine
{
    std::shared_ptr<SceneData> Scene::createSceneData(uint32_t emitting_object_count)
    {
        auto scene_data = std::make_shared<SceneData>();

        std::shared_ptr<Camera> camera =
            SceneUtil::collectCameras(getRootNode()).at(0); // TODO remove this by having a own camera uniform buffer
        scene_data->inverse_view = camera->getInverseView();
        scene_data->inverse_proj = camera->getInverseProjection();
        scene_data->view_pos = glm::vec4(camera->getPosition(), 0.0F);

        std::array<glm::vec4, POINT_LIGHT_COUNT> point_light_positions = {};
        std::array<glm::vec4, POINT_LIGHT_COUNT> point_light_colors = {};
        for (uint32_t i = 0; i < POINT_LIGHT_COUNT; i++)
        {
            point_light_positions[i] = glm::vec4{pointLights[i].position, pointLights[i].intensity};
            point_light_colors[i] = glm::vec4{pointLights[i].color, 0.0F};
        }

        scene_data->pointLightPositions = point_light_positions;
        scene_data->pointLightColors = point_light_colors;
        scene_data->sunlightDirection = glm::vec4(sun.direction, sun.intensity);
        scene_data->sunlightColor = glm::vec4(sun.color, 0.0F);
        scene_data->sunlightColor = glm::vec4{1, 0, 0, 1.0F};

        scene_data->ambientColor = glm::vec4(0.05F);

        scene_data->emitting_object_count = emitting_object_count;

        return scene_data;
    }

    void Scene::addNode(const std::string& name, std::shared_ptr<Node> node)
    {
        assert(!nodes.contains(name));
        nodes[name] = std::move(node);
    }

    std::shared_ptr<Node> Scene::getRootNode()
    {
        return nodes["root"];
    }

    void Scene::start()
    {
        for (auto& node : nodes)
        {
            node.second->start();
        }
    }

    void Scene::update()
    {
        getRootNode()->refreshTransform(glm::mat4(1.0F));

        for (auto& node : nodes)
        {
            node.second->update();
        }
    }

    void Scene::destroy()
    {
        for (auto& node : nodes)
        {
            node.second->destroy();
        }
    }

    std::vector<std::shared_ptr<MeshAsset>> Scene::getMeshAssets()
    {
        return SceneUtil::collectMeshAssets(getRootNode());
    }

    std::vector<std::shared_ptr<MaterialInstance>> Scene::getMaterialInstances()
    {
        return SceneUtil::collectMaterialInstances(getRootNode());
    }

    void Scene::fillDrawContext(const std::shared_ptr<DrawContext>& draw_context)
    {
        getRootNode()->draw(*draw_context);
    }

    std::shared_ptr<Material> Scene::getMaterial()
    {
        return material;
    }

    std::shared_ptr<EnvironmentMap> Scene::getEnvironmentMap()
    {
        return environment_map;
    }

    void* Scene::getSceneData(size_t* size, uint32_t emitting_instances_count)
    {
        last_scene_data = createSceneData(emitting_instances_count);
        *size = sizeof(SceneData);
        return last_scene_data.get();
    }

} // namespace RtEngine
