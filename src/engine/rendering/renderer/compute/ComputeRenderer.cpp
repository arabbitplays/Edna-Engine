#include "compute/ComputeRenderer.hpp"

#include <glitch.comp.spv.h>

#include "DescriptorLayoutBuilder.hpp"

namespace RtEngine {
    ComputeRenderer::ComputeRenderer(const std::shared_ptr<VulkanContext> &vulkan_context,
                                     const std::shared_ptr<SyncManager> &sync_manager,
                                     const uint32_t max_frames_in_flight)
        : Renderer(vulkan_context, sync_manager, max_frames_in_flight) {
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


    void ComputeRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, std::shared_ptr<RenderTarget> target, uint32_t swapchain_image_idx) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getHandle());
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getLayoutHandle(), 0, 1, &descriptor_set, 0, 0);
        recordDispatch(commandBuffer, target);
    }

    void ComputeRenderer::submitCommandBuffer(uint32_t stage_index) {
        submitStage(stage_index, vulkan_context->device_manager->getQueue(COMPUTE));
    }

    void ComputeRenderer::cleanup() {
        deletion_queue.flush();
    }
} // RtEngine