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
#include "Luna.hpp"
#include "ShaderModule.hpp"

namespace luna
{
ComputePipeline::ComputePipeline(const VkDevice device, const LunaComputePipelineCreationInfo &creationInfo)
{
    CHECK_RESULT_THROW(helpers::createPipelineLayout(device,
                                                     creationInfo.layoutCreationInfo,
                                                     pushConstantsRanges_,
                                                     &layout_));

    const VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .flags = creationInfo.shaderStageCreationInfo.flags,
        .stage = creationInfo.shaderStageCreationInfo.stage,
        .module = helpers::fromHandle<ShaderModule>(creationInfo.shaderStageCreationInfo.module)->module(),
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

void ComputePipeline::destroy(const VkDevice device)
{
    vkDestroyPipeline(device, pipeline_, nullptr);
    vkDestroyPipelineLayout(device, layout_, nullptr);

    pushConstantsRanges_.clear();
    pushConstantsRanges_.shrink_to_fit();
}

VkResult ComputePipeline::bind(const VkCommandBuffer commandBuffer,
                               const LunaDescriptorSetBindInfo &descriptorSetBindInfo) const
{
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

VkResult lunaCreateComputePipeline(const LunaDevice device,
                                   const LunaComputePipelineCreationInfo *creationInfo,
                                   LunaComputePipeline *pipeline)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo);
    luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    CHECK_RESULT_RETURN(deviceObject.createComputePipeline(static_cast<VkDevice>(deviceObject),
                                                           *creationInfo,
                                                           pipeline));
    return VK_SUCCESS;
}

VkResult lunaDispatch(const LunaDevice device, const LunaCommandBuffer commandBuffer, const LunaDispatchInfo *info)
{
    assert(info);
    const LunaDispatchBaseInfo baseInfo = {
        .pipeline = info->pipeline,
        .descriptorSetBindInfo = info->descriptorSetBindInfo,
        .groupCountX = info->groupCountX,
        .groupCountY = info->groupCountY,
        .groupCountZ = info->groupCountZ,
        .submitInfo = info->submitInfo,
    };
    return lunaDispatchBase(device, commandBuffer, &baseInfo);
}
VkResult lunaDispatchBase(const LunaDevice device,
                          const LunaCommandBuffer commandBuffer,
                          const LunaDispatchBaseInfo *info)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(info && info->pipeline != LUNA_NULL_HANDLE);

    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);

    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(static_cast<VkDevice>(deviceObject)));
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::ComputePipeline>(info->pipeline)
                                ->bind(commandBufferObject,
                                       info->descriptorSetBindInfo == nullptr ? LunaDescriptorSetBindInfo{}
                                                                              : *info->descriptorSetBindInfo));
    vkCmdDispatchBase(commandBufferObject,
                      info->baseGroupX,
                      info->baseGroupY,
                      info->baseGroupZ,
                      info->groupCountX == 0 ? 1 : info->groupCountX,
                      info->groupCountY == 0 ? 1 : info->groupCountY,
                      info->groupCountZ == 0 ? 1 : info->groupCountZ);
    if (info->submitInfo != nullptr)
    {
        CHECK_RESULT_RETURN(commandBufferObject.endAndSubmit(static_cast<VkDevice>(*luna::helpers::fromHandle<
                                                                                   luna::Device>(device)),
                                                             *info->submitInfo));
    }
    return VK_SUCCESS;
}
VkResult lunaBindComputePipeline(const VkCommandBuffer commandBuffer,
                                 const LunaComputePipeline pipeline,
                                 const LunaDescriptorSetBindInfo *descriptorSetBindInfo)
{
    assert(pipeline != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::ComputePipeline>(pipeline)->bind(
            commandBuffer,
            descriptorSetBindInfo == nullptr ? LunaDescriptorSetBindInfo{} : *descriptorSetBindInfo));
    return VK_SUCCESS;
}
