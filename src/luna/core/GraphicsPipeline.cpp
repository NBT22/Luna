//
// Created by NBT22 on 2/25/25.
//

#include <luna/core/GraphicsPipeline.hpp>
#include <luna/core/Instance.hpp>
#include <luna/lunaPipeline.h>

namespace luna::helpers
{
VkResult createPipelineLayout(const LunaPipelineLayoutCreationInfo &layoutCreationInfo,
                              std::vector<LunaPushConstantsRange> &pushConstantsRanges,
                              VkPipelineLayout *layout)
{
    const uint32_t descriptorSetLayoutCount = layoutCreationInfo.descriptorSetLayoutCount;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.reserve(descriptorSetLayoutCount);
    for (uint32_t i = 0; i < descriptorSetLayoutCount; i++)
    {
        descriptorSetLayouts.emplace_back(core::descriptorSetLayout(layoutCreationInfo.descriptorSetLayouts[i]));
    }
    uint32_t pushConstantsOffset = 0;
    std::vector<VkPushConstantRange> pushConstantRanges;
    pushConstantRanges.reserve(layoutCreationInfo.pushConstantRangeCount);
    for (uint32_t i = 0; i < layoutCreationInfo.pushConstantRangeCount; i++)
    {
        const LunaPushConstantsRange &pushConstantsRange = layoutCreationInfo.pushConstantsRanges[i];
        pushConstantsRanges.push_back(pushConstantsRange);
        pushConstantRanges.emplace_back(pushConstantsRange.stageFlags, pushConstantsOffset, pushConstantsRange.size);
        pushConstantsOffset += pushConstantsRange.size;
    }
    const VkPipelineLayoutCreateInfo layoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .flags = layoutCreationInfo.flags,
        .setLayoutCount = descriptorSetLayoutCount,
        .pSetLayouts = descriptorSetLayouts.data(),
        .pushConstantRangeCount = layoutCreationInfo.pushConstantRangeCount,
        .pPushConstantRanges = pushConstantRanges.data(),
    };
    return vkCreatePipelineLayout(core::device, &layoutCreateInfo, nullptr, layout);
}
} // namespace luna::helpers

namespace luna::core
{
// TODO: Base pipeline
GraphicsPipeline::GraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo)
{
    assert(isDestroyed_);
    assert(!(creationInfo.shaderStageCount > 0 && // NOLINT(*-simplify-boolean-expr) In order to preserve clarity
             creationInfo.shaderStages == nullptr));

    CHECK_RESULT_THROW(helpers::createPipelineLayout(creationInfo.layoutCreationInfo, pushConstantsRanges_, &layout_));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(creationInfo.shaderStageCount);
    for (uint32_t i = 0; i < creationInfo.shaderStageCount; i++)
    {
        // I literally have an assert to ensure it isn't
        // ReSharper disable once CppDFANullDereference
        const LunaPipelineShaderStageCreationInfo shaderStage = creationInfo.shaderStages[i];
        shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                  nullptr,
                                  shaderStage.flags,
                                  shaderStage.stage,
                                  device.shaderModule(shaderStage.module),
                                  shaderStage.entrypoint == nullptr ? "main" : shaderStage.entrypoint,
                                  shaderStage.specializationInfo);
    }

    const RenderPassSubpassIndex *subpassIndex = static_cast<const RenderPassSubpassIndex *>(creationInfo.subpass);
    const VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .flags = creationInfo.flags,
        .stageCount = creationInfo.shaderStageCount,
        .pStages = shaderStages.data(),
        .pVertexInputState = creationInfo.vertexInputState,
        .pInputAssemblyState = creationInfo.inputAssemblyState,
        .pTessellationState = creationInfo.tessellationState,
        .pViewportState = creationInfo.viewportState,
        .pRasterizationState = creationInfo.rasterizationState,
        .pMultisampleState = creationInfo.multisampleState,
        .pDepthStencilState = creationInfo.depthStencilState,
        .pColorBlendState = creationInfo.colorBlendState,
        .pDynamicState = creationInfo.dynamicState,
        .layout = layout_,
        .renderPass = renderPass(subpassIndex->renderPassIndex),
        .subpass = subpassIndex->index,
    };
    CHECK_RESULT_THROW(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline_));

    isDestroyed_ = false;
}
void GraphicsPipeline::destroy()
{
    if (isDestroyed_)
    {
        return;
    }
    vkDestroyPipeline(device, pipeline_, nullptr);
    vkDestroyPipelineLayout(device, layout_, nullptr);

    pushConstantsRanges_.clear();
    pushConstantsRanges_.shrink_to_fit();
    isDestroyed_ = true;
}
VkResult GraphicsPipeline::bind(const LunaGraphicsPipelineBindInfo &bindInfo) const
{
    if (pipeline_ == boundPipeline)
    {
        return VK_SUCCESS;
    }
    CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    if (!commandBuffer.isRecording())
    {
        CHECK_RESULT_RETURN(commandBuffer.waitForFence(device));
        CHECK_RESULT_RETURN(commandBuffer.beginSingleUseCommandBuffer());
    }
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
    if (bindInfo.descriptorSetCount > 0)
    {
        std::vector<VkDescriptorSet> descriptorSets;
        descriptorSets.reserve(bindInfo.descriptorSetCount);
        for (uint32_t i = 0; i < bindInfo.descriptorSetCount; i++)
        {
            descriptorSets.emplace_back(descriptorSet(bindInfo.descriptorSets[i]));
        }
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout_,
                                bindInfo.firstSet,
                                bindInfo.descriptorSetCount,
                                descriptorSets.data(),
                                bindInfo.dynamicOffsetCount,
                                bindInfo.dynamicOffsets);
    }
    boundPipeline = pipeline_;
    return VK_SUCCESS;
}
} // namespace luna::core

VkResult lunaCreateGraphicsPipeline(const LunaGraphicsPipelineCreationInfo *creationInfo,
                                    LunaGraphicsPipeline *pipeline)
{
    using namespace luna::core;
    assert(creationInfo);
    const std::vector<GraphicsPipeline>::iterator &pipelineIterator = std::find_if(graphicsPipelines.begin(),
                                                                                   graphicsPipelines.end(),
                                                                                   GraphicsPipeline::isDestroyed);
    graphicsPipelineIndices.emplace_back(pipelineIterator - graphicsPipelines.begin());
    TRY_CATCH_RESULT(graphicsPipelines.emplace(pipelineIterator, *creationInfo));
    if (pipeline != nullptr)
    {
        *pipeline = &graphicsPipelineIndices.back();
    }
    return VK_SUCCESS;
}

void lunaBindDescriptorSets(const LunaGraphicsPipeline pipeline, const LunaGraphicsPipelineBindInfo *bindInfo)
{
    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.reserve(bindInfo->descriptorSetCount);
    for (uint32_t i = 0; i < bindInfo->descriptorSetCount; i++)
    {
        descriptorSets.emplace_back(luna::core::descriptorSet(bindInfo->descriptorSets[i]));
    }
    const uint32_t pipelineIndex = static_cast<const luna::core::GraphicsPipelineIndex *>(pipeline)->index;
    vkCmdBindDescriptorSets(luna::core::device.commandPools().graphics.commandBuffer(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            luna::core::graphicsPipelines.at(pipelineIndex).layout(),
                            bindInfo->firstSet,
                            bindInfo->descriptorSetCount,
                            descriptorSets.data(),
                            bindInfo->dynamicOffsetCount,
                            bindInfo->dynamicOffsets);
}

VkResult lunaPushConstants(const LunaGraphicsPipeline pipeline)
{
    const uint32_t pipelineIndex = static_cast<const luna::core::GraphicsPipelineIndex *>(pipeline)->index;
    const luna::core::GraphicsPipeline &graphicsPipeline = luna::core::graphicsPipelines.at(pipelineIndex);
    const std::vector<LunaPushConstantsRange> &pushConstantsRanges = graphicsPipeline.pushConstantsRanges_;
    luna::core::CommandBuffer &commandBuffer = luna::core::device.commandPools().graphics.commandBuffer();
    if (!commandBuffer.isRecording())
    {
        CHECK_RESULT_RETURN(commandBuffer.waitForFence(luna::core::device));
        CHECK_RESULT_RETURN(commandBuffer.beginSingleUseCommandBuffer());
    }
    uint32_t offset = 0;
    for (const LunaPushConstantsRange &pushConstantsRange: pushConstantsRanges)
    {
        const void *pushConstantsData = static_cast<const uint8_t *>(pushConstantsRange.dataPointer) +
                                        pushConstantsRange.dataPointerOffset;
        vkCmdPushConstants(commandBuffer,
                           graphicsPipeline.layout_,
                           pushConstantsRange.stageFlags,
                           offset,
                           pushConstantsRange.size,
                           pushConstantsData);
        offset += pushConstantsRange.size;
    }
    return VK_SUCCESS;
}
