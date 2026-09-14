#ifndef EDNA_ENGINE_CYCLICALCELLULARAUTOMATONRENDERER_HPP
#define EDNA_ENGINE_CYCLICALCELLULARAUTOMATONRENDERER_HPP
#include <array>
#include <glm/glm.hpp>

#include "BufferConnector.hpp"
#include "ComputeRenderer.hpp"
#include "ImageConnector.hpp"

namespace RtEngine {
    class CyclicalCellularAutomatonRenderer : public ComputeRenderer {
    public:
        CyclicalCellularAutomatonRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                                          VkExtent2D image_extent,
                                          const std::vector<glm::vec4>& colors,
                                          uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

        void setThreshold(uint32_t threshold) { push.threshold = threshold; }
        void setUpdate(bool update) { push.update = update ? 1u : 0u; }

        void handleResize(VkExtent2D new_extent);

    protected:
        VkShaderModule createShaderModule() override;

        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;

    private:
        struct PushConstants {
            uint32_t threshold;
            uint32_t update;
        };

        void initializeState();

        VkExtent2D image_extent;
        std::vector<glm::vec4> colors;

        PushConstants push{3u, 0u};

        std::shared_ptr<ImageConnector> state_connector;
        std::shared_ptr<ImageConnector> target_connector;
        std::shared_ptr<BufferConnector> palette_connector;
    };
} // RtEngine

#endif //EDNA_ENGINE_CYCLICALCELLULARAUTOMATONRENDERER_HPP
