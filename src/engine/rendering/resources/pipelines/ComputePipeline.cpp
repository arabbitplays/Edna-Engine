#include "ComputePipeline.hpp"

namespace RtEngine
{
    void ComputePipeline::build()
    {
        VkDevice device = context->device_manager->getDevice();

        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkComputePipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_info.stage = shader_stage;
        pipeline_info.layout = layout;
        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &handle) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create compute pipeline!");
        }

        deletionQueue.pushFunction(
            [&]()
            {
                vkDestroyPipelineLayout(context->device_manager->getDevice(), layout, nullptr);
                vkDestroyPipeline(context->device_manager->getDevice(), handle, nullptr);
            });
    }

    void ComputePipeline::setShaderStage(VkShaderModule shader_module)
    {
        shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shader_stage.module = shader_module;
        shader_stage.pName = "main";
    }

    void ComputePipeline::setDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
    {
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptor_set_layouts.data();
    }

    void ComputePipeline::addPushConstant(uint32_t size, VkShaderStageFlags shader_stage)
    {
        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = shader_stage;
        push_constant_range.offset = 0;
        push_constant_range.size = size;

        pushConstants.push_back(push_constant_range);
    }

    void ComputePipeline::clear()
    {
        pipelineLayoutInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pushConstants.clear();
    }

    void ComputePipeline::destroy()
    {
        deletionQueue.flush();
    }

    VkPipeline ComputePipeline::getHandle() const
    {
        return handle;
    }

    VkPipelineLayout ComputePipeline::getLayoutHandle() const
    {
        return layout;
    }

} // namespace RtEngine