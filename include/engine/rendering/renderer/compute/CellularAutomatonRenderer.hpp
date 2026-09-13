#ifndef EDNA_ENGINE_CELLULARAUTOMATONRENDERER_HPP
#define EDNA_ENGINE_CELLULARAUTOMATONRENDERER_HPP
#include <glm/glm.hpp>
#include <vector>

#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"

namespace RtEngine {
    class CellularAutomatonRenderer : public ComputeRenderer {
    public:
        CellularAutomatonRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                           VkExtent2D image_extent,
                           std::vector<glm::vec4> colors,
                           uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

    protected:
        VkShaderModule createShaderModule() override;

        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;

    private:
        std::shared_ptr<ImageConnector> output_connector;
        std::vector<glm::vec4> colors;
        size_t color_index = 0;
    };
} // RtEngine

#endif //EDNA_ENGINE_CELLULARAUTOMATONRENDERER_HPP
