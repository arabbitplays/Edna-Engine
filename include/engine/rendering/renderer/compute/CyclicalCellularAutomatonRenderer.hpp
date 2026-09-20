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
        static constexpr uint32_t MAX_STATE_COUNT    = 64;
        static constexpr uint32_t MAX_NEIGHBOR_COUNT = 256;

        CyclicalCellularAutomatonRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                                          VkExtent2D image_extent,
                                          const std::vector<glm::vec4>& colors,
                                          const std::vector<glm::ivec2>& neighbor_offsets,
                                          uint32_t max_frames_in_flight = 1);

        std::shared_ptr<ImageConnector> getOutputConnector() const;

        void setThreshold(uint32_t threshold) { push.threshold = threshold; }
        void setUpdate(bool update) { push.update = update ? 1u : 0u; }
        void setUpdateChance(float chance) { push.update_chance = chance; }
        void setMutationChance(float chance) { push.mutation_chance = chance; }

        void setPalette(const std::vector<glm::vec4>& new_colors);
        void setNeighborhood(const std::vector<glm::ivec2>& new_offsets);

        void handleResize(VkExtent2D new_extent);

    protected:
        VkShaderModule createShaderModule() override;

        void configurePushConstants(ComputePipeline& pipeline) override;
        void recordPushConstants(VkCommandBuffer cmd) override;

    private:
        struct PushConstants {
            uint32_t state_count;
            uint32_t threshold;
            uint32_t update;
            float    update_chance;
            float    mutation_chance;
        };

        void initializeState();

        VkExtent2D image_extent;
        std::vector<glm::vec4> colors;

        PushConstants push{0u, 1u, 0u, 1.0f, 0.0f};

        static VkDeviceSize neighborhoodBufferSize();

        std::shared_ptr<ImageConnector> state_connector;
        std::shared_ptr<ImageConnector> target_connector;
        std::shared_ptr<ImageConnector> rng_connector;
        std::shared_ptr<BufferConnector> palette_connector;
        std::shared_ptr<BufferConnector> neighborhood_connector;
    };
} // RtEngine

#endif //EDNA_ENGINE_CYCLICALCELLULARAUTOMATONRENDERER_HPP
