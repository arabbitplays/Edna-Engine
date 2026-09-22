#ifndef EDNA_ENGINE_MANDELBULBRENDERER_HPP
#define EDNA_ENGINE_MANDELBULBRENDERER_HPP
#include "BufferConnector.hpp"
#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace RtEngine
{
    class MandelbulbRenderer : public ComputeRenderer
    {
    public:
        static constexpr uint32_t MAX_COLOR_COUNT = 256;

        MandelbulbRenderer(const std::shared_ptr<VulkanContext>& vulkan_context, VkExtent2D image_extent,
            const std::vector<glm::vec4>& colors, const glm::vec3& initial, float power, uint32_t max_iterations,
            uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

        void setCamera(const glm::mat4& inverse_view, const glm::mat4& inverse_projection)
        {
            camera_data.inverse_view = inverse_view;
            camera_data.inverse_projection = inverse_projection;
        }
        void setRotation(const glm::mat4& rotation)
        {
            camera_data.rotation = rotation;
        }
        void setInitial(const glm::vec3& initial)
        {
            push.initial = glm::vec4(initial, 0.0f);
        }
        void setPower(float power)
        {
            push.power = power;
        }
        void setMaxIterations(uint32_t iterations)
        {
            push.max_iterations = iterations;
        }

        void setPalette(const std::vector<glm::vec4>& new_colors);

        void handleResize(VkExtent2D new_extent);

    protected:
        VkShaderModule createShaderModule() override;

        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;
        void recordPreDispatch(VkCommandBuffer cmd) override;

    private:
        struct PushConstants
        {
            glm::vec4 initial;
            float power;
            uint32_t max_iterations;
            uint32_t color_count;
            uint32_t _padding;
        };

        struct CameraData
        {
            glm::mat4 inverse_view;
            glm::mat4 inverse_projection;
            glm::mat4 rotation;
        };

        VkExtent2D image_extent;
        std::vector<glm::vec4> colors;

        PushConstants push{glm::vec4(0.0f), 8.0f, 8u, 0u, 0u};
        CameraData camera_data{glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f)};

        std::shared_ptr<ImageConnector> target_connector;
        std::shared_ptr<BufferConnector> palette_connector;
        std::shared_ptr<BufferConnector> camera_connector;
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_MANDELBULBRENDERER_HPP
