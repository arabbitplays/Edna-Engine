#ifndef VULKAN_RAYTRACING_GLITCHRENDERER_HPP
#define VULKAN_RAYTRACING_GLITCHRENDERER_HPP
#include <chrono>

#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"


namespace RtEngine {
    class GlitchRenderer : public ComputeRenderer {
    public:
        GlitchRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                       VkExtent2D image_extent,
                       std::shared_ptr<ImageConnector> input_connector,
                       uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

        void setShakePower(float v)      { push.shake_power = v; }
        void setShakeRate(float v)       { push.shake_rate = v; }
        void setShakeSpeed(float v)      { push.shake_speed = v; }
        void setShakeBlockSize(float v)  { push.shake_block_size = v; }
        void setShakeColorRate(float v)  { push.shake_color_rate = v; }

    protected:
        VkShaderModule createShaderModule() override;

        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;

    private:
        struct PushConstants {
            float time;
            float shake_power;
            float shake_rate;
            float shake_speed;
            float shake_block_size;
            float shake_color_rate;
        };

        std::shared_ptr<ImageConnector> output_connector;

        PushConstants push{0.0f, 0.03f, 0.2f, 5.0f, 30.5f, 0.01f};
        std::chrono::steady_clock::time_point start_time;
    };
}
#endif //VULKAN_RAYTRACING_GLITCHRENDERER_HPP
