//
// Created by NBT22 on 10/18/25.
//

#pragma once

#include <cstdint>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "DescriptorSetLayout.hpp"
#include "helpers/Handle.hpp"
#include "Instance.hpp"

namespace luna::helpers
{
static inline VkResult createPipelineLayout(const VkDevice device,
                                            const LunaPipelineLayoutCreationInfo &layoutCreationInfo,
                                            std::vector<LunaPushConstantsRange> &pushConstantsRanges,
                                            VkPipelineLayout *layout)
{
    const uint32_t descriptorSetLayoutCount = layoutCreationInfo.descriptorSetLayoutCount;
    std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
    vkDescriptorSetLayouts.reserve(descriptorSetLayoutCount);
    for (uint32_t i = 0; i < descriptorSetLayoutCount; i++)
    {
        vkDescriptorSetLayouts.emplace_back(
                fromHandle<DescriptorSetLayout>(layoutCreationInfo.descriptorSetLayouts[i])->layout());
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
        .pSetLayouts = vkDescriptorSetLayouts.data(),
        .pushConstantRangeCount = layoutCreationInfo.pushConstantRangeCount,
        .pPushConstantRanges = pushConstantRanges.data(),
    };
    return vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, layout);
}
} // namespace luna::helpers
