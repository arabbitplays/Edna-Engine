#ifndef EDNA_ENGINE_MANDELBROTRENDERER_HPP
#define EDNA_ENGINE_MANDELBROTRENDERER_HPP
#include <glm/glm.hpp>
#include <vector>

#include "BufferConnector.hpp"
#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"

namespace RtEngine {
    class MandelbrotRenderer : public ComputeRenderer {
    public:
        static constexpr uint32_t MAX_COLOR_COUNT = 256;

        MandelbrotRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                           VkExtent2D image_extent,
                           const std::vector<glm::vec4>& colors,
                           double origin_x,
                           double origin_y,
                           double offset_x,
                           double offset_y,
                           double step_size,
                           uint32_t max_iterations,
                           double initial_x,
                           double initial_y,
                           bool julia_mode,
                           uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

        // Origin is an absolute world/complex-plane coordinate; the view is
        // centered on it when the offset is (0,0).
        void setOrigin(double x, double y) { world_origin_x = x; world_origin_y = y; }
        // Offset is in "object space": each unit shifts the view by one
        // screen width/height at the current step_size and image extent.
        void setOffset(double x, double y) { screen_offset_x = x; screen_offset_y = y; }
        void setStepSize(double step) { push.step_size = step; }
        void setMaxIterations(uint32_t iterations) { push.max_iterations = iterations; }
        void setInitial(double x, double y) { push.initial_x = x; push.initial_y = y; }
        void setJuliaMode(bool julia) { push.julia_mode = julia ? 1u : 0u; }

        void setPalette(const std::vector<glm::vec4>& new_colors);

        void handleResize(VkExtent2D new_extent);

    protected:
        VkShaderModule createShaderModule() override;

        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;

    private:
        struct PushConstants {
            double   origin_x;
            double   origin_y;
            double   step_size;
            double   initial_x;
            double   initial_y;
            uint32_t max_iterations;
            uint32_t color_count;
            uint32_t julia_mode;
            uint32_t _padding;
        };

        VkExtent2D image_extent;
        std::vector<glm::vec4> colors;

        double world_origin_x = 0.0;
        double world_origin_y = 0.0;
        double screen_offset_x = 0.0;
        double screen_offset_y = 0.0;

        PushConstants push{0.0, 0.0, 0.003, 0.0, 0.0, 256u, 0u, 0u, 0u};

        std::shared_ptr<ImageConnector>  target_connector;
        std::shared_ptr<BufferConnector> palette_connector;
    };
} // RtEngine

#endif //EDNA_ENGINE_MANDELBROTRENDERER_HPP
