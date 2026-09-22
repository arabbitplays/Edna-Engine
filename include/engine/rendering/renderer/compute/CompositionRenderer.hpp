#ifndef EDNA_ENGINE_COMPOSITIONRENDERER_HPP
#define EDNA_ENGINE_COMPOSITIONRENDERER_HPP
#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"

namespace RtEngine {
    class CompositionRenderer : public ComputeRenderer {
    public:
        CompositionRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                            VkExtent2D image_extent,
                            std::shared_ptr<ImageConnector> input_a,
                            std::shared_ptr<ImageConnector> input_b,
                            uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

        void setFade(float fade) { push.fade = fade; }
        void setInvertColor(bool invert) { push.invert_color = invert ? 1u : 0u; }

        void handleResize(VkExtent2D new_extent);

    protected:
        VkShaderModule createShaderModule() override;

        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;

    private:
        struct PushConstants {
            float    fade;
            uint32_t invert_color;
        };

        PushConstants push{0.0f, 0u};

        std::shared_ptr<ImageConnector> output_connector;
    };
}
#endif //EDNA_ENGINE_COMPOSITIONRENDERER_HPP
