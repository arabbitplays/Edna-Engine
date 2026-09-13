#include "compute/ComputeRenderer.hpp"

#include <cassert>
#include <stdexcept>

namespace RtEngine {
    ComputeRenderer::ComputeRenderer(const std::shared_ptr<VulkanContext> &vulkan_context,
                                     const uint32_t max_frames_in_flight)
        : Renderer(vulkan_context, max_frames_in_flight),
          connector_layout(std::make_shared<ConnectorLayout>(vulkan_context->device_manager,
                                                             vulkan_context->descriptor_allocator)) {
    }

    void ComputeRenderer::init() {
        Renderer::init();
        createPipeline();
    }

    void ComputeRenderer::addConnector(uint32_t binding, ConnectorHandle connector) {
        connector_layout->addConnector(binding, std::move(connector));
    }

    void ComputeRenderer::setDispatchSize(VkExtent3D size) {
        dispatch_size_provider = [size]() { return size; };
    }

    void ComputeRenderer::setDispatchSize(DispatchSizeProvider provider) {
        dispatch_size_provider = std::move(provider);
    }

    ConnectorHandle ComputeRenderer::getConnector(uint32_t binding) const {
        return connector_layout->getConnectors()[binding];
    }

    void ComputeRenderer::createPipeline() {
        pipeline = std::make_shared<ComputePipeline>(vulkan_context);
        VkDevice device = vulkan_context->device_manager->getDevice();

        descriptor_layout = connector_layout->createLayout(VK_SHADER_STAGE_COMPUTE_BIT);
        deletion_queue.pushFunction([&]() {
            vkDestroyDescriptorSetLayout(vulkan_context->device_manager->getDevice(), descriptor_layout, nullptr);
        });

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{descriptor_layout};
        pipeline->setDescriptorSetLayouts(descriptorSetLayouts);

        VkShaderModule compute_shader_module = createShaderModule();
        pipeline->setShaderStage(compute_shader_module);

        pipeline->build();

        deletion_queue.pushFunction([&]() { pipeline->destroy(); });

        vkDestroyShaderModule(device, compute_shader_module, nullptr);
    }


    VkCommandBuffer ComputeRenderer::recordCommandBuffer(uint32_t frame_idx) {
        VkCommandBuffer cmd = getFreshCommandBuffer(frame_idx);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("ComputeRenderer: failed to begin command buffer");
        }

        descriptor_set = connector_layout->writeConnectors(descriptor_layout);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayoutHandle(), 0, 1, &descriptor_set, 0, 0);

        assert(dispatch_size_provider && "ComputeRenderer: dispatch size not set");
        VkExtent3D dispatch_size = dispatch_size_provider();
        vkCmdDispatch(cmd, dispatch_size.width, dispatch_size.height, dispatch_size.depth);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            throw std::runtime_error("ComputeRenderer: failed to end command buffer");
        }
        return cmd;
    }
} // RtEngine
