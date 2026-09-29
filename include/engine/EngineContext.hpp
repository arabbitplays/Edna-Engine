#ifndef RUNTIMECONTEXT_HPP
#define RUNTIMECONTEXT_HPP

#include <engine/InputManager.hpp>
#include <engine/scene_graph/ISceneManager.hpp>
#include <engine/rendering/renderer/RaytracingRenderer.hpp>
#include <engine/RenderingManager.hpp>
#include <engine/SwapchainManager.hpp>
#include <engine/SyncManager.hpp>

#include <engine/repositories/MeshRepository.hpp>
#include <engine/repositories/TextureRepository.hpp>

namespace RtEngine
{
    class Material;

    struct EngineContext
    {
        std::shared_ptr<Window> window;
        std::shared_ptr<SwapchainManager> swapchain_manager;
        std::shared_ptr<SyncManager> sync_manager;

        std::shared_ptr<RenderingManager> rendering_manager;
        std::shared_ptr<TextureRepository> texture_repository;
        std::shared_ptr<MeshRepository> mesh_repository;

        std::shared_ptr<ISceneManager> scene_manager;
        std::shared_ptr<InputManager> input_manager;
    };
} // namespace RtEngine

#endif // RUNTIMECONTEXT_HPP
