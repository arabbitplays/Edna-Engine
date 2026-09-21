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
    };
}

#endif //VULKAN_RAYTRACING_ISCENEMANAGER_HPP
