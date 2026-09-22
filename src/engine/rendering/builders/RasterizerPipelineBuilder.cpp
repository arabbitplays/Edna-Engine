#include "Vertex.hpp"

#include <RasterizerPipelineBuilder.hpp>
#include <string>

namespace RtEngine
{
    void RasterizerPipelineBuilder::buildPipeline(const VkDevice& device, const VkRenderPass& render_pass,
        VkPipeline* pipeline, const VkPipelineLayout& pipeline_layout)
    {

        std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
        dynamic_state.pDynamicStates = dynamic_states.data();

        auto binding_description = Vertex::getBindingDescription();
        auto attribute_descriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.pVertexBindingDescriptions = &binding_description;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<int32_t>(attribute_descriptions.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        // viewport and scissor is dynamic state and thus set later

        rasterizerInfo.depthClampEnable = VK_FALSE;
        rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizerInfo.depthBiasEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &colorBlendAttachment;

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipeline_info.pStages = shaderStages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &inputAssemblyInfo;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizerInfo;
        pipeline_info.pMultisampleState = &multisamplingInfo;
        pipeline_info.pDepthStencilState = &depthStencilInfo;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_state;
        pipeline_info.layout = pipeline_layout;
        pipeline_info.renderPass = render_pass;
        pipeline_info.subpass = 0;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    void RasterizerPipelineBuilder::buildPipelineLayout(const VkDevice& device, VkPipelineLayout* pipeline_layout) const
    {
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pipeline_layout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void RasterizerPipelineBuilder::setShaders(VkShaderModule vert_shader_module, VkShaderModule frag_shader_module)
    {
        VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
        vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_stage_info.module = vert_shader_module;
        vert_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
        frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_stage_info.module = frag_shader_module;
        frag_shader_stage_info.pName = "main";

        shaderStages.push_back(vert_shader_stage_info);
        shaderStages.push_back(frag_shader_stage_info);
    }

    void RasterizerPipelineBuilder::setInputTopology(const VkPrimitiveTopology topology)
    {
        inputAssemblyInfo.topology = topology;
    }

    void RasterizerPipelineBuilder::setPolygonMode(VkPolygonMode polygon_mode)
    {
        rasterizerInfo.polygonMode = polygon_mode;
        rasterizerInfo.lineWidth = 1.0F;
    }

    void RasterizerPipelineBuilder::setCullMode(const VkCullModeFlags cull_mode, const VkFrontFace front_face)
    {
        rasterizerInfo.cullMode = cull_mode;
        rasterizerInfo.frontFace = front_face;
    }

    void RasterizerPipelineBuilder::setMultisamplingNone()
    {
        multisamplingInfo.sampleShadingEnable = VK_FALSE;
        multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    void RasterizerPipelineBuilder::enableDepthTest(const VkBool32 enabled, const VkCompareOp compare_op)
    {
        depthStencilInfo.depthTestEnable = enabled;
        depthStencilInfo.depthWriteEnable = enabled;
        depthStencilInfo.depthCompareOp = compare_op;
        depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    }

    void RasterizerPipelineBuilder::disableColorBlending()
    {
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
    }

    void RasterizerPipelineBuilder::enableAdditiveBlending()
    {
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;

        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    }

    void RasterizerPipelineBuilder::setDescriptorSetLayouts(
        const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
    {
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptor_set_layouts.data();
    }

    void RasterizerPipelineBuilder::setPushConstantRanges(const std::vector<VkPushConstantRange>& ranges)
    {
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
        pipelineLayoutInfo.pPushConstantRanges = ranges.data();
    }

    void RasterizerPipelineBuilder::clear()
    {
        inputAssemblyInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        rasterizerInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        multisamplingInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        depthStencilInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        colorBlendAttachment = {};
        pipelineLayoutInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        shaderStages.clear();
    }
} // namespace RtEngine
