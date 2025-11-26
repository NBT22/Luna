//
// Created by NBT22 on 2/25/25.
//

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <luna/luna.h>
#include <luna/lunaTypes.h>
#include <spirv_reflect.h>
#include <stdexcept>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"
#include "GraphicsPipeline.hpp"
#include "helpers/Handle.hpp"
#include "helpers/Pipeline.hpp"
#include "Instance.hpp"
#include "Luna.hpp"
#include "RenderPass.hpp"

namespace luna
{
// TODO (0.3.0): Base pipeline & SPIRV reflection
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
        const LunaPipelineShaderStageCreationInfo &shaderStage = creationInfo.shaderStages[i];
        shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                  nullptr,
                                  shaderStage.flags,
                                  shaderStage.stage,
                                  *helpers::fromHandle<ShaderModule>(shaderStage.module),
                                  shaderStage.entryPoint == nullptr ? "main" : shaderStage.entryPoint,
                                  shaderStage.specializationInfo);
    }

    const RenderPassSubpassIndex *subpassIndex = helpers::fromHandle<RenderPassSubpassIndex>(creationInfo.subpass);
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
        .renderPass = *subpassIndex->renderPass,
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
VkResult GraphicsPipeline::pushConstants() const
{
    const std::vector<LunaPushConstantsRange> &pushConstantsRanges = pushConstantsRanges_;
    CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(luna::device));
    uint32_t offset = 0;
    for (const LunaPushConstantsRange &pushConstantsRange: pushConstantsRanges)
    {
        const void *pushConstantsData = static_cast<const uint8_t *>(pushConstantsRange.dataPointer) +
                                        pushConstantsRange.dataPointerOffset;
        vkCmdPushConstants(commandBuffer,
                           layout_,
                           pushConstantsRange.stageFlags,
                           offset,
                           pushConstantsRange.size,
                           pushConstantsData);
        offset += pushConstantsRange.size;
    }
    return VK_SUCCESS;
}
VkResult GraphicsPipeline::bind(const LunaGraphicsPipelineBindInfo &bindInfo) const
{
    CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(device));
    for (uint32_t i = 0; i < bindInfo.dynamicStateCount; i++)
    {
        const LunaDynamicStateBindInfo &dynamicState = bindInfo.dynamicStates[i];
        switch (dynamicState.dynamicStateType)
        {
            case VK_DYNAMIC_STATE_VIEWPORT:
                vkCmdSetViewport(commandBuffer,
                                 dynamicState.bindInfo.viewportBindInfo->firstViewport,
                                 dynamicState.bindInfo.viewportBindInfo->viewportCount,
                                 dynamicState.bindInfo.viewportBindInfo->viewports);
                break;
            case VK_DYNAMIC_STATE_SCISSOR:
                vkCmdSetScissor(commandBuffer,
                                dynamicState.bindInfo.scissorBindInfo->firstScissor,
                                dynamicState.bindInfo.scissorBindInfo->scissorCount,
                                dynamicState.bindInfo.scissorBindInfo->scissors);
                break;
            default:
                throw std::runtime_error("Unhandled dynamic state type!");
        }
    }
    if (pipeline_ != boundPipeline)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
    }
    if (bindInfo.descriptorSetBindInfo.descriptorSetCount > 0)
    {
        std::vector<VkDescriptorSet> descriptorSetsVector;
        descriptorSetsVector.reserve(bindInfo.descriptorSetBindInfo.descriptorSetCount);
        for (uint32_t i = 0; i < bindInfo.descriptorSetBindInfo.descriptorSetCount; i++)
        {
            descriptorSetsVector.emplace_back(
                    *helpers::fromHandle<DescriptorSetIndex>(bindInfo.descriptorSetBindInfo.descriptorSets[i])->set);
        }
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout_,
                                bindInfo.descriptorSetBindInfo.firstSet,
                                bindInfo.descriptorSetBindInfo.descriptorSetCount,
                                descriptorSetsVector.data(),
                                bindInfo.descriptorSetBindInfo.dynamicOffsetCount,
                                bindInfo.descriptorSetBindInfo.dynamicOffsets);
    }
    boundPipeline = pipeline_;
    return VK_SUCCESS;
}
} // namespace luna

VkResult lunaCreateGraphicsPipeline(const LunaGraphicsPipelineCreationInfo *creationInfo,
                                    LunaGraphicsPipeline *pipeline)
{
    assert(creationInfo);
    TRY_CATCH_RESULT(luna::graphicsPipelines.emplace_back(*creationInfo));
    if (pipeline != nullptr)
    {
        *pipeline = &luna::graphicsPipelines.back();
    }
    return VK_SUCCESS;
}
VkResult lunaCreateGraphicsPipelineUsingReflection(const LunaGraphicsPipelineUsingReflectionCreationInfo *creationInfo,
                                                   LunaGraphicsPipeline *pipeline)
{
    (void)pipeline;

    using namespace luna;

    std::vector<LunaShaderModule> shaderModules;
    std::vector<const char *> entryPoints;
    if (creationInfo->shaderModuleCreationInfoCount != 0)
    {
        assert(creationInfo->shaderModuleCount <= creationInfo->shaderModuleCreationInfoCount);

        shaderModules.resize(creationInfo->shaderModuleCreationInfoCount);
        for (uint32_t i = 0; i < creationInfo->shaderModuleCreationInfoCount; i++)
        {
            lunaCreateShaderModule(creationInfo->shaderModuleCreationInfos + i, shaderModules.data() + i);
            entryPoints.emplace_back(creationInfo->entryPoints == nullptr || creationInfo->entryPoints[i] == nullptr
                                             ? "main"
                                             : creationInfo->entryPoints[i]);
        }
        for (uint32_t i = 0; i < creationInfo->shaderModuleCount; i++)
        {
            creationInfo->shaderModules[i] = shaderModules.at(i);
        }
    } else
    {
        shaderModules.insert(shaderModules.cbegin(),
                             creationInfo->shaderModules,
                             creationInfo->shaderModules + creationInfo->shaderModuleCount);
        for (uint32_t i = 0; i < creationInfo->shaderModuleCount; i++)
        {
            entryPoints.emplace_back(creationInfo->entryPoints[i] == nullptr ? "main" : creationInfo->entryPoints[i]);
        }
    }
    std::vector<LunaPipelineShaderStageCreationInfo> shaderStages;
    shaderStages.reserve(shaderModules.size());
    for (uint32_t shaderModuleIndex = 0; shaderModuleIndex < shaderModules.size(); shaderModuleIndex++)
    {
        const ShaderModule &lunaShaderModule = *helpers::fromHandle<ShaderModule>(shaderModules.at(shaderModuleIndex));
        SpvReflectShaderModule module = {};
        spvReflectCreateShaderModule(lunaShaderModule.size(),
                                     lunaShaderModule.spirv().data(),
                                     &module); // TODO (0.3.0): Handle return value
        SpvReflectEntryPoint entryPoint;
        for (uint32_t entryPointIndex = 0; entryPointIndex < module.entry_point_count; entryPointIndex++)
        {
            if (std::strcmp(module.entry_points[entryPointIndex].name, entryPoints.at(shaderModuleIndex)) == 0)
            {
                entryPoint = module.entry_points[entryPointIndex];
                shaderStages.emplace_back(0,
                                          static_cast<VkShaderStageFlagBits>(entryPoint.shader_stage),
                                          lunaShaderModule.module(),
                                          entryPoint.name,
                                          nullptr);
                break;
            }
        }
        asm("nop");
    }
    return VK_SUCCESS;
}

void lunaBindDescriptorSets(const LunaGraphicsPipeline pipeline, const LunaDescriptorSetBindInfo *bindInfo)
{
    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.reserve(bindInfo->descriptorSetCount);
    for (uint32_t i = 0; i < bindInfo->descriptorSetCount; i++)
    {
        descriptorSets.emplace_back(
                *luna::helpers::fromHandle<luna::DescriptorSetIndex>(bindInfo->descriptorSets[i])->set);
    }
    vkCmdBindDescriptorSets(luna::device.commandPools().graphics.commandBuffer(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            luna::helpers::fromHandle<luna::GraphicsPipeline>(pipeline)->layout(),
                            bindInfo->firstSet,
                            bindInfo->descriptorSetCount,
                            descriptorSets.data(),
                            bindInfo->dynamicOffsetCount,
                            bindInfo->dynamicOffsets);
}

VkResult lunaPushConstants(const LunaGraphicsPipeline pipeline)
{
    return luna::helpers::fromHandle<luna::GraphicsPipeline>(pipeline)->pushConstants();
}
