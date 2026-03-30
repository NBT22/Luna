//
// Created by NBT22 on 10/18/25.
//

#include <cassert>
#include <cstdint>
#include <luna/luna.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"
#include "ComputePipeline.hpp"
#include "helpers/Handle.hpp"
#include "helpers/Pipeline.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna
{
ComputePipeline::ComputePipeline(const LunaComputePipelineCreationInfo &creationInfo)
{
    CHECK_RESULT_THROW(helpers::createPipelineLayout(creationInfo.layoutCreationInfo, pushConstantsRanges_, &layout_));

    const VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .flags = creationInfo.shaderStageCreationInfo.flags,
        .stage = creationInfo.shaderStageCreationInfo.stage,
        .module = *helpers::fromHandle<ShaderModule>(creationInfo.shaderStageCreationInfo.module),
        .pName = creationInfo.shaderStageCreationInfo.entryPoint == nullptr
                         ? "main"
                         : creationInfo.shaderStageCreationInfo.entryPoint,
        .pSpecializationInfo = creationInfo.shaderStageCreationInfo.specializationInfo,
    };

    const VkComputePipelineCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .flags = creationInfo.flags,
        .stage = shaderStageCreateInfo,
        .layout = layout_,
    };
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_);
}

ComputePipeline::~ComputePipeline()
{
    vkDestroyPipeline(device, pipeline_, nullptr);
    vkDestroyPipelineLayout(device, layout_, nullptr);

    pushConstantsRanges_.clear();
    pushConstantsRanges_.shrink_to_fit();
}

VkResult ComputePipeline::bind(const LunaDescriptorSetBindInfo &descriptorSetBindInfo) const
{
    CommandBuffer &commandBuffer = device.commandPools().compute->commandBuffer();
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording());

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);
    if (descriptorSetBindInfo.descriptorSetCount > 0)
    {
        std::vector<VkDescriptorSet> descriptorSetsVector;
        descriptorSetsVector.reserve(descriptorSetBindInfo.descriptorSetCount);
        for (uint32_t i = 0; i < descriptorSetBindInfo.descriptorSetCount; i++)
        {
            descriptorSetsVector.emplace_back(
                    *helpers::fromHandle<DescriptorSetIndex>(descriptorSetBindInfo.descriptorSets[i])->set);
        }
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                layout_,
                                descriptorSetBindInfo.firstSet,
                                descriptorSetBindInfo.descriptorSetCount,
                                descriptorSetsVector.data(),
                                descriptorSetBindInfo.dynamicOffsetCount,
                                descriptorSetBindInfo.dynamicOffsets);
    }
    return VK_SUCCESS;
}
} // namespace luna

VkResult lunaCreateComputePipeline(const LunaComputePipelineCreationInfo *creationInfo, LunaComputePipeline *pipeline)
{
    assert(creationInfo);
    TRY_CATCH_RESULT(luna::computePipelines.emplace_back(*creationInfo));
    if (pipeline != nullptr)
    {
        *pipeline = luna::helpers::toHandle(&luna::computePipelines.back());
    }
    return VK_SUCCESS;
}

VkResult lunaDispatchBase(const LunaDispatchBaseInfo *info)
{
    assert(info);
    assert(info->pipeline != LUNA_NULL_HANDLE);
    luna::CommandBuffer &commandBuffer = luna::device.commandPools().compute->commandBuffer();
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(true));
    CHECK_RESULT_RETURN(
            luna::helpers::fromHandle<luna::ComputePipeline>(info->pipeline)->bind(info->descriptorSetBindInfo));
    vkCmdDispatchBase(commandBuffer,
                      info->baseGroupX,
                      info->baseGroupY,
                      info->baseGroupZ,
                      info->groupCountX == 0 ? 1 : info->groupCountX,
                      info->groupCountY == 0 ? 1 : info->groupCountY,
                      info->groupCountZ == 0 ? 1 : info->groupCountZ);
    if (info->submitCommandBuffer)
    {
        // TODO (0.3.0): If possible, optionally allow the source stage mask
        //  (if not possible to be optional, just use top of pipe)
        CHECK_RESULT_RETURN(commandBuffer.submitCommandBuffer(luna::device.familyQueues().compute,
                                                              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT));
    }
    return VK_SUCCESS;
}
VkResult lunaDispatch(const LunaDispatchInfo *info)
{
    assert(info);
    const LunaDispatchBaseInfo baseInfo = {
        .pipeline = info->pipeline,
        .descriptorSetBindInfo = info->descriptorSetBindInfo,
        .groupCountX = info->groupCountX,
        .groupCountY = info->groupCountY,
        .groupCountZ = info->groupCountZ,
        .submitCommandBuffer = info->submitCommandBuffer,
    };
    return lunaDispatchBase(&baseInfo);
}
