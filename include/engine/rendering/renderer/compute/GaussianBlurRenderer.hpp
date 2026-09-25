#ifndef EDNA_ENGINE_GAUSSIANBLURRENDERER_HPP
#define EDNA_ENGINE_GAUSSIANBLURRENDERER_HPP
#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"

#include <cstdint>

namespace RtEngine
{
    // One axis of a separable Gaussian blur. Chain two instances (Horizontal
    // then Vertical) to produce a full 2D blur. radius == 0 makes the pass a
    // pass-through copy, so the pipeline can stay wired while the effect is
    // disabled.
    class GaussianBlurRenderer : public ComputeRenderer
    {
    public:
        enum class Direction
        {
            Horizontal,
            Vertical,
        };

        static constexpr uint32_t MAX_RADIUS = 64u;

        GaussianBlurRenderer(const std::shared_ptr<VulkanContext>& vulkan_context, VkExtent2D image_extent,
            std::shared_ptr<ImageConnector> input_connector, Direction direction,
            uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

        // sigma is derived from radius so the ±radius kernel always covers
        // ±3σ (no visible truncation regardless of the chosen size).
        void setRadius(uint32_t radius);

        void handleResize(VkExtent2D new_extent);

    protected:
        VkShaderModule createShaderModule() override;
        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;

    private:
        struct PushConstants
        {
            int32_t direction_x;
            int32_t direction_y;
            uint32_t radius;
            float sigma;
        };

        std::shared_ptr<ImageConnector> output_connector;
        PushConstants push{1, 0, 0u, 1.0f};
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_GAUSSIANBLURRENDERER_HPP
