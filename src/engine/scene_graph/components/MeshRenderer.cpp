#include "MeshRenderer.hpp"

#include <Node.hpp>
#include <Scene.hpp>

namespace RtEngine
{
    void MeshRenderer::OnStart()
    {
        mesh_asset = context->mesh_repository->getMesh(mesh_asset_name);
        assert(mesh_asset != nullptr);

        std::shared_ptr<Material> material = context->scene_manager->getCurrentMaterial();
        mesh_material = material->getInstanceByName(material_instance_name);
        assert(mesh_material != nullptr);
    }

    void MeshRenderer::OnRender(DrawContext& ctx)
    {
        auto shared_node = node.lock();
        if (!shared_node)
        {
            assert(false);
        }

        glm::mat4 node_matrix = shared_node->transform->getWorldTransform();

        ctx.addRenderObject(
            RenderObject{.instance_mapping_data = InstanceMappingData{.geometry_id = mesh_asset->geometry_id,
                             .material_index = mesh_material->getMaterialIndex()},
                .acceleration_structure = mesh_asset->accelerationStructure,
                .transform = node_matrix,
                .primitive_count = mesh_asset->triangle_count,
                .emitting_power = mesh_material->getEmissionPower()});
    }

    void MeshRenderer::initProperties(const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& update_flags)
    {

        if (config->startChild(COMPONENT_NAME))
        {
            config->addString("mesh", &mesh_asset_name);
            config->addString("material_name", &material_instance_name);

            if (mesh_material)
            {
                mesh_material->initProperties(config, update_flags);
            }
            config->endChild();
        }
    }

} // namespace RtEngine
