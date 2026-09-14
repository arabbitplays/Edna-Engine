//
// Created by oschdi on 18.01.26.
//

#ifndef VULKAN_RAYTRACING_SCENEMANAGER_HPP
#define VULKAN_RAYTRACING_SCENEMANAGER_HPP
#include <memory>
#include <unordered_map>

#include "Camera.hpp"
#include "ISceneManager.hpp"
#include "Scene.hpp"

namespace RtEngine {
    struct EngineContext;
    class SceneReader;

    class SceneManager : public ISceneManager {
    public:
        SceneManager(const std::string &resources_dir,
                     const std::shared_ptr<DeviceManager> &device_manager,
                     std::shared_ptr<SceneReader> scene_reader);

        std::shared_ptr<Scene> getCurrentScene();
        void setScene(const std::shared_ptr<Scene> &new_scene);
        std::shared_ptr<Material> getCurrentMaterial() override;

        std::string getScenePath(std::string scene_name);
        std::vector<std::string> getSceneNames() const;

        std::shared_ptr<Scene> loadScene(const std::string &scene_path,
                                         std::unordered_map<std::string, std::shared_ptr<Material>> materials = {});

        void destroy() const;
    private:
        std::string resources_dir;
        std::shared_ptr<Scene> scene;
        std::vector<std::string> scene_names;

        std::shared_ptr<DeviceManager> device_manager;
        std::shared_ptr<SceneReader> scene_reader;
    };
} // RtEngine

#endif //VULKAN_RAYTRACING_SCENEMANAGER_HPP