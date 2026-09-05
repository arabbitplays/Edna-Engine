#include "compute/ComputeRenderer.hpp"

#include <glitch.comp.spv.h>
#include <stdexcept>

#include "DescriptorLayoutBuilder.hpp"

namespace RtEngine {
    ComputeRenderer::ComputeRenderer(const std::shared_ptr<VulkanContext> &vulkan_context,
                                     const uint32_t max_frames_in_flight)
        : Renderer(vulkan_context, max_frames_in_flight) {
    }

    void ComputeRenderer::init() {
        Renderer::init();
        createPipeline();
    }

    void ComputeRenderer::createPipeline() {
        pipeline = std::make_shared<ComputePipeline>(vulkan_context);
        VkDevice device = vulkan_context->device_manager->getDevice();

        DescriptorLayoutBuilder layoutBuilder;
        initDescriptorLayout(layoutBuilder);
        descriptor_layout = layoutBuilder.build(device, VK_SHADER_STAGE_COMPUTE_BIT);
        deletion_queue.pushFunction([&]() {
            vkDestroyDescriptorSetLayout(vulkan_context->device_manager->getDevice(), descriptor_layout, nullptr);
        });
        descriptor_set = vulkan_context->descriptor_allocator->allocate(vulkan_context->device_manager->getDevice(), descriptor_layout);

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

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayoutHandle(), 0, 1, &descriptor_set, 0, 0);
        recordDispatch(cmd, current_target);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            throw std::runtime_error("ComputeRenderer: failed to end command buffer");
        }
        return cmd;
    }

    void ComputeRenderer::cleanup() {
        deletion_queue.flush();
    }
} // RtEngine