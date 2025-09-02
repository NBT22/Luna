//
// Created by NBT22 on 3/17/25.
//

#include <cassert>
#include <cstdint>
#include <luna/luna.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "DescriptorSetLayout.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna::core
{
DescriptorSetLayout::DescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo &creationInfo)
{
    assert(isDestroyed_);
    std::vector<VkDescriptorBindingFlags> bindingFlags;
    bindingFlags.reserve(creationInfo.bindingCount);
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(creationInfo.bindingCount);
    for (uint32_t i = 0; i < creationInfo.bindingCount; i++)
    {
        const LunaDescriptorSetLayoutBinding &binding = creationInfo.bindings[i];
        assert(!bindingMap_.contains(binding.bindingName));
        const uint32_t bindingIndex = bindingMap_.size();
        bindingMap_.emplace(binding.bindingName, Binding{.index = bindingIndex, .type = binding.descriptorType});
        bindings.emplace_back(bindingIndex,
                              binding.descriptorType,
                              binding.descriptorCount,
                              binding.stageFlags,
                              binding.immutableSamplers);
        bindingFlags.emplace_back(binding.bindingFlags);
    }
    const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = creationInfo.bindingCount,
        .pBindingFlags = bindingFlags.data(),
    };
    const VkDescriptorSetLayoutCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = VK_API_VERSION_MINOR(apiVersion) >= 2 ? &bindingFlagsCreateInfo : nullptr,
        .flags = creationInfo.flags,
        .bindingCount = creationInfo.bindingCount,
        .pBindings = bindings.data(),
    };
    CHECK_RESULT_THROW(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout_));
    isDestroyed_ = false;
}

void DescriptorSetLayout::destroy()
{
    if (isDestroyed_)
    {
        return;
    }
    vkDestroyDescriptorSetLayout(device, layout_, nullptr);
    bindingMap_.clear();
    isDestroyed_ = true;
}
} // namespace luna::core

VkResult lunaCreateDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo *creationInfo,
                                       LunaDescriptorSetLayout *descriptorSetLayout)
{
    using namespace luna::core;
    assert(creationInfo);
    TRY_CATCH_RESULT(descriptorSetLayouts.emplace_back(*creationInfo));
    if (descriptorSetLayout != nullptr)
    {
        *descriptorSetLayout = &descriptorSetLayouts.back();
    }
    return VK_SUCCESS;
}
