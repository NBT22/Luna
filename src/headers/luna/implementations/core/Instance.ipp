//
// Created by NBT22 on 2/15/25.
//

#pragma once

namespace luna::core
{
inline void Instance::unbindAllPipelines()
{
	for (GraphicsPipeline &pipeline: graphicsPipelines_)
	{
		pipeline.unbind();
	}
}
inline void Instance::addNewDevice(const LunaDeviceCreationInfo2 &creationInfo)
{
	device_ = Device(creationInfo);
}
inline const RenderPassIndex *Instance::createRenderPass(const LunaRenderPassCreationInfo *creationInfo)
{
	renderPassIndices_.emplace_back(renderPasses_.size());
	renderPasses_.emplace_back(creationInfo, nullptr, &renderPassIndices_.back());
	return &renderPassIndices_.back();
}
inline const RenderPassIndex *Instance::createRenderPass(const LunaRenderPassCreationInfo2 *creationInfo2)
{
	renderPassIndices_.emplace_back(renderPasses_.size());
	const LunaRenderPassCreationInfo creationInfo = {
		.samples = creationInfo2->samples,
		.createColorAttachment = creationInfo2->createColorAttachment,
		.colorAttachmentLoadMode = creationInfo2->colorAttachmentLoadMode,
		.createDepthAttachment = creationInfo2->createDepthAttachment,
		.depthAttachmentLoadMode = creationInfo2->depthAttachmentLoadMode,
		.attachmentCount = creationInfo2->attachmentCount,
		.subpassCount = creationInfo2->subpassCount,
		.subpassNames = creationInfo2->subpassNames,
		.dependencyCount = creationInfo2->dependencyCount,
		.extent = creationInfo2->extent,
		.framebufferAttachmentCount = creationInfo2->framebufferAttachmentCount,
	};
	renderPasses_.emplace_back(&creationInfo, creationInfo2, &renderPassIndices_.back());
	return &renderPassIndices_.back();
}
inline const DescriptorPoolIndex *Instance::createDescriptorPool(const LunaDescriptorPoolCreationInfo &creationInfo)
{
	descriptorPoolIndices_.emplace_back(descriptorPools_.size());
	descriptorPools_.emplace_back();
	const VkDescriptorPoolCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = creationInfo.flags,
		.maxSets = creationInfo.maxSets,
		.poolSizeCount = creationInfo.poolSizeCount,
		.pPoolSizes = creationInfo.poolSizes,
	};
	vkCreateDescriptorPool(device_.logicalDevice(), &createInfo, nullptr, &descriptorPools_.back());
	return &descriptorPoolIndices_.back();
}
inline const DescriptorSetLayoutIndex *Instance::createDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo
																				   &creationInfo)
{
	descriptorSetLayoutIndices_.emplace_back(descriptorSetLayouts_.size());
	descriptorSetLayouts_.emplace_back(creationInfo);
	return &descriptorSetLayoutIndices_.back();
}
inline void Instance::allocateDescriptorSets(const LunaDescriptorSetAllocationInfo &allocationInfo,
											 LunaDescriptorSet *descriptorSets)
{
	std::vector<VkDescriptorSetLayout> layouts;
	layouts.reserve(allocationInfo.descriptorSetCount);
	for (uint32_t i = 0; i < allocationInfo.descriptorSetCount; i++)
	{
		const LunaDescriptorSetLayout layout = allocationInfo.setLayouts[i];
		layouts.emplace_back(descriptorSetLayout(layout).layout());

		descriptorSetIndices_.emplace_back(descriptorSets_.size() + i,
										   static_cast<const DescriptorSetLayoutIndex *>(layout),
										   static_cast<const DescriptorPoolIndex *>(allocationInfo.descriptorPool));
		descriptorSets[i] = &descriptorSetIndices_.back();
	}
	const VkDescriptorSetAllocateInfo allocateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool(allocationInfo.descriptorPool),
		.descriptorSetCount = allocationInfo.descriptorSetCount,
		.pSetLayouts = layouts.data(),
	};
	const size_t oldSize = descriptorSets_.size();
	descriptorSets_.resize(oldSize + allocationInfo.descriptorSetCount);
	vkAllocateDescriptorSets(device_.logicalDevice(), &allocateInfo, descriptorSets_.data() + oldSize);
}
inline const GraphicsPipelineIndex *Instance::createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo
																			 &creationInfo)
{
	const RenderPassIndex *index = static_cast<const RenderPassSubpassIndex *>(creationInfo.subpass)->renderPassIndex;
	// TODO: Create a method on RenderPass to do this that can make RenderPass::pipelineIndices private again (and make it take a LunaRenderPassSubpass so that we don't need index)
	renderPass_(index).pipelineIndices.emplace_back(graphicsPipelines_.size());

	graphicsPipelineIndices_.emplace_back(graphicsPipelines_.size());
	graphicsPipelines_.emplace_back(creationInfo);
	return &graphicsPipelineIndices_.back();
}
inline uint32_t Instance::allocateBuffer(const LunaBufferCreationInfo &creationInfo)
{
	const VkBufferCreateInfo bufferCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = creationInfo.flags,
		.size = creationInfo.size,
		.usage = creationInfo.usage,
		.sharingMode = device().sharingMode(),
		.queueFamilyIndexCount = device().familyCount(),
		.pQueueFamilyIndices = device().queueFamilyIndices(),
	};
	buffers_.emplace_back(bufferCreateInfo);
	return buffers_.size() - 1;
}
inline void Instance::createStagingBuffer(const size_t size)
{
	const LunaBufferCreationInfo bufferCreationInfo = {
		.size = size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	};
	stagingBuffer_ = buffer::BufferRegion::createBuffer(bufferCreationInfo);
}
inline void Instance::copyToStagingBuffer(const uint8_t *data, const size_t size) const
{
	bufferRegion(stagingBuffer_).copyToBuffer(data, size);
}
inline const SamplerIndex *Instance::createSampler(const LunaSamplerCreationInfo &creationInfo)
{
	samplerIndices_.emplace_back(samplers_.size());
	samplers_.emplace_back();
	const VkSamplerCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.flags = creationInfo.flags,
		.magFilter = creationInfo.magFilter,
		.minFilter = creationInfo.minFilter,
		.mipmapMode = creationInfo.mipmapMode,
		.addressModeU = creationInfo.addressModeU,
		.addressModeV = creationInfo.addressModeV,
		.addressModeW = creationInfo.addressModeW,
		.mipLodBias = creationInfo.mipLodBias,
		.anisotropyEnable = creationInfo.anisotropyEnable,
		.maxAnisotropy = creationInfo.maxAnisotropy,
		.compareEnable = creationInfo.compareEnable,
		.compareOp = creationInfo.compareOp,
		.minLod = creationInfo.minLod,
		.maxLod = creationInfo.maxLod,
		.borderColor = creationInfo.borderColor,
		.unnormalizedCoordinates = creationInfo.unnormalizedCoordinates,
	};
	vkCreateSampler(device_.logicalDevice(), &createInfo, nullptr, &samplers_.back());
	return &samplerIndices_.back();
}

inline uint32_t Instance::minorVersion() const
{
	return VK_API_VERSION_MINOR(apiVersion_);
}
inline VkInstance Instance::instance() const
{
	return instance_;
}
inline const Device &Instance::device() const
{
	return device_;
}
inline FamilyValues<CommandBuffer> &Instance::commandBuffers()
{
	return device_.commandBuffers();
}
inline const FamilyValues<CommandBuffer> &Instance::commandBuffers() const
{
	return device_.commandBuffers();
}
inline VkSurfaceKHR Instance::surface() const
{
	return surface_;
}
inline const RenderPass &Instance::renderPass(const LunaRenderPass renderPass) const
{
	return renderPasses_.at(static_cast<const RenderPassIndex *>(renderPass)->index);
}
inline VkDescriptorPool Instance::descriptorPool(const LunaDescriptorPool descriptorPool) const
{
	return descriptorPools_.at(static_cast<const DescriptorPoolIndex *>(descriptorPool)->index);
}
inline const DescriptorSetLayout &Instance::descriptorSetLayout(const LunaDescriptorSetLayout layout) const
{
	return descriptorSetLayouts_.at(static_cast<const DescriptorSetLayoutIndex *>(layout)->index);
}
inline VkDescriptorSet Instance::descriptorSet(const LunaDescriptorSet descriptorSet) const
{
	return descriptorSets_.at(static_cast<const DescriptorSetIndex *>(descriptorSet)->index);
}
inline void Instance::descriptorSet(const LunaDescriptorSet index,
									VkDescriptorPool *pool,
									DescriptorSetLayout *layout,
									VkDescriptorSet *descriptorSet) const
{
	const DescriptorSetIndex descriptorSetIndex = *static_cast<const DescriptorSetIndex *>(index);
	if (pool)
	{
		*pool = descriptorPool(descriptorSetIndex.poolIndex);
	}
	if (layout)
	{
		*layout = descriptorSetLayout(descriptorSetIndex.layoutIndex);
	}
	if (descriptorSet)
	{
		*descriptorSet = descriptorSets_.at(descriptorSetIndex.index);
	}
}
inline const buffer::BufferRegion &Instance::bufferRegion(const LunaBuffer buffer) const
{
	const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(buffer);
	return buffers_.at(index.bufferIndex).region(index.bufferRegionIndex);
}
inline const buffer::BufferRegion &Instance::bufferRegion(const buffer::BufferRegionIndex index) const
{
	return buffers_.at(index.bufferIndex).region(index.bufferRegionIndex);
}
inline VkBuffer Instance::stagingBuffer() const
{
	const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(stagingBuffer_);
	return buffers_.at(index.bufferIndex).buffer();
}
inline size_t Instance::stagingBufferOffset() const
{
	if (stagingBuffer_ == nullptr)
	{
		return -1ull;
	}
	return bufferRegion(stagingBuffer_).offset();
}
inline VkSampler Instance::sampler(const LunaSampler sampler) const
{
	return samplers_.at(static_cast<const SamplerIndex *>(sampler)->index);
}
inline VkSampler Instance::sampler(const SamplerIndex *sampler) const
{
	return samplers_.at(sampler->index);
}

inline RenderPass &Instance::renderPass_(const LunaRenderPass index)
{
	return renderPasses_.at(static_cast<const RenderPassIndex *>(index)->index);
}
} // namespace luna::core
