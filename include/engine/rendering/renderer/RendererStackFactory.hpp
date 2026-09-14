#ifndef EDNA_ENGINE_RENDERERSTACKFACTORY_HPP
#define EDNA_ENGINE_RENDERERSTACKFACTORY_HPP
#include <memory>

#include "ImageConnector.hpp"
#include "MeshRepository.hpp"
#include "PresentStage.hpp"
#include "RaytracingRenderer.hpp"
#include "RendererStack.hpp"
#include "TextureRepository.hpp"
#include "VulkanContext.hpp"

namespace RtEngine {
    // Builds a fully wired RendererStack for one of the supported engine modes.
    // The stack is returned with its present stage + present connector already
    // set; the caller is only responsible for driving frame submission.
    class RendererStackFactory {
    public:
        struct Result {
            std::shared_ptr<RendererStack> stack;
            // Raytracing-only outputs. Both are null when produced by createComputeStack().
            std::shared_ptr<RaytracingRenderer> raytracing_renderer;
            std::shared_ptr<ImageConnector> raytracing_target_connector;
        };

        RendererStackFactory(std::shared_ptr<VulkanContext> vulkan_context,
                             std::shared_ptr<PresentStage> present_stage,
                             std::shared_ptr<MeshRepository> mesh_repository,
                             std::shared_ptr<TextureRepository> texture_repository,
                             uint32_t max_frames_in_flight);

        Result createEmptyStack();
        Result createRaytracingStack();

    private:
        std::shared_ptr<RendererStack> makeStack() const;

        std::shared_ptr<VulkanContext> vulkan_context;
        std::shared_ptr<PresentStage> present_stage;
        std::shared_ptr<MeshRepository> mesh_repository;
        std::shared_ptr<TextureRepository> texture_repository;
        uint32_t max_frames_in_flight;
    };
} // RtEngine

#endif //EDNA_ENGINE_RENDERERSTACKFACTORY_HPP
