#ifndef EDNA_ENGINE_MANDELBROTRENDERER_HPP
#define EDNA_ENGINE_MANDELBROTRENDERER_HPP
#include "BufferConnector.hpp"
#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace RtEngine
{
    class MandelbrotRenderer : public ComputeRenderer
    {
    public:
        static constexpr uint32_t MAX_COLOR_COUNT = 256;

        MandelbrotRenderer(const std::shared_ptr<VulkanContext>& vulkan_context, VkExtent2D image_extent,
            const std::vector<glm::vec4>& colors, float origin_x, float origin_y, float offset_x, float offset_y,
            float step_size, uint32_t max_iterations, float initial_x, float initial_y, bool julia_mode,
            uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

        // Complex-plane coordinate the view centers on when offset is (0,0).
        void setOrigin(float x, float y)
        {
            world_origin_x = x;
            world_origin_y = y;
        }
        // Each offset unit shifts the view by one screen width/height at the
        // current step_size and image extent.
        void setOffset(float x, float y)
        {
            screen_offset_x = x;
            screen_offset_y = y;
        }
        void setStepSize(float step)
        {
            push.step_size = step;
        }
        void setMaxIterations(uint32_t iterations)
        {
            push.max_iterations = iterations;
        }
        void setInitial(float x, float y)
        {
            push.initial_x = x;
            push.initial_y = y;
        }
        void setJuliaMode(bool julia)
        {
            push.julia_mode = julia ? 1u : 0u;
        }

        void setPalette(const std::vector<glm::vec4>& new_colors);

        void handleResize(VkExtent2D new_extent);

    protected:
        VkShaderModule createShaderModule() override;

        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;

    private:
        struct PushConstants
        {
            float origin_x;
            float origin_y;
            float step_size;
            float initial_x;
            float initial_y;
            uint32_t max_iterations;
            uint32_t color_count;
            uint32_t julia_mode;
        };

        VkExtent2D image_extent;
        std::vector<glm::vec4> colors;

        float world_origin_x = 0.0f;
        float world_origin_y = 0.0f;
        float screen_offset_x = 0.0f;
        float screen_offset_y = 0.0f;

        PushConstants push{0.0f, 0.0f, 0.003f, 0.0f, 0.0f, 256u, 0u, 0u};

        std::shared_ptr<ImageConnector> target_connector;
        std::shared_ptr<BufferConnector> palette_connector;
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_MANDELBROTRENDERER_HPP
