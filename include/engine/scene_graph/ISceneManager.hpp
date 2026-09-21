#ifndef VULKAN_RAYTRACING_ISCENEMANAGER_HPP
#define VULKAN_RAYTRACING_ISCENEMANAGER_HPP
#include <memory>

#include "Material.hpp"

namespace RtEngine {
    class Scene;

    class ISceneManager {
    public:
        virtual ~ISceneManager() = default;

        virtual std::shared_ptr<Material> getCurrentMaterial() = 0;
        virtual std::shared_ptr<Scene> getCurrentScene() = 0;

        // Requires Scene.hpp at the instantiation site.
        template<typename T>
        std::shared_ptr<T> getComponent();
    };

    template<typename T>
    std::shared_ptr<T> ISceneManager::getComponent() {
        const auto scene = getCurrentScene();
        if (!scene) return nullptr;
        for (const auto& [name, node] : scene->nodes) {
            if (auto comp = node->template getComponent<T>()) return comp;
        }
        return nullptr;
    }
}

#endif //VULKAN_RAYTRACING_ISCENEMANAGER_HPP
