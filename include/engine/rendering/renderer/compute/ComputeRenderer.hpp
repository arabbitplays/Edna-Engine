#ifndef VULKAN_RAYTRACING_COMPUTERENDERER_HPP
#define VULKAN_RAYTRACING_COMPUTERENDERER_HPP
#include <functional>
#include <memory>

#include "ComputePipeline.hpp"
#include "ConnectorLayout.hpp"
#include "../Renderer.hpp"
#include "VulkanContext.hpp"

namespace RtEngine {
    class ComputeRenderer : public Renderer {
    public:
        using DispatchSizeProvider = std::function<VkExtent3D()>;

        ComputeRenderer(const std::shared_ptr<VulkanContext>& vulkan_context,
                        const uint32_t max_frames_in_flight = 1);

        void init() override;

        void addConnector(uint32_t binding, ConnectorHandle connector);
        void setDispatchSize(VkExtent3D size);
        void setDispatchSize(DispatchSizeProvider provider);

        VkCommandBuffer recordCommandBuffer(uint32_t frame_idx) override;
        QueueType queueType() const override { return COMPUTE; }

        void cleanup();
    protected:
        void createPipeline();
        virtual VkShaderModule createShaderModule() = 0;

        ConnectorHandle getConnector(uint32_t binding) const;

        std::shared_ptr<ComputePipeline> pipeline;
        std::shared_ptr<ConnectorLayout> connector_layout;
        VkDescriptorSetLayout descriptor_layout;
        VkDescriptorSet descriptor_set;

        DispatchSizeProvider dispatch_size_provider;
    };
} // RtEngine

#endif //VULKAN_RAYTRACING_COMPUTERENDERER_HPP
