//
// Created by NBT22 on 3/17/25.
//

#include <luna/core/DescriptorSetLayout.hpp>
#include <luna/core/Instance.hpp>
#include <luna/luna.h>

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
		const LunaDescriptorSetLayoutBinding binding = creationInfo.bindings[i];
		assert(!bindingIndexMap_.contains(binding.bindingName));
		const uint32_t bindingIndex = bindingIndexMap_.size();
		bindingIndexMap_.emplace(binding.bindingName, (Binding){.index = bindingIndex, .type = binding.descriptorType});
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
		.pNext = instance.minorVersion() >= 2 ? &bindingFlagsCreateInfo : nullptr,
		.flags = creationInfo.flags,
		.bindingCount = creationInfo.bindingCount,
		.pBindings = bindings.data(),
	};
	vkCreateDescriptorSetLayout(instance.device().logicalDevice(), &createInfo, nullptr, &layout_);
	isDestroyed_ = false;
}

void DescriptorSetLayout::destroy()
{
	if (isDestroyed_)
	{
		return;
	}
	vkDestroyDescriptorSetLayout(instance.device().logicalDevice(), layout_, nullptr);
	bindingIndexMap_.clear();
	isDestroyed_ = true;
}
} // namespace luna::core

LunaDescriptorSetLayout lunaCreateDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo *creationInfo)
{
	assert(creationInfo);
	return luna::core::instance.createDescriptorSetLayout(*creationInfo);
}
