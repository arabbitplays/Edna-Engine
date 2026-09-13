#ifndef VULKAN_RAYTRACING_GLITCHRENDERER_HPP
#define VULKAN_RAYTRACING_GLITCHRENDERER_HPP
#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"

namespace RtEngine {
    class GlitchRenderer : public ComputeRenderer {
    public:
        GlitchRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                       VkExtent2D image_extent,
                       std::shared_ptr<ImageConnector> input_connector,
                       const uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

    protected:
        VkShaderModule createShaderModule() override;

    private:
        std::shared_ptr<ImageConnector> output_connector;
    };
}
#endif //VULKAN_RAYTRACING_GLITCHRENDERER_HPP
