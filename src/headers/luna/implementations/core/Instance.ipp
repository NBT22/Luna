//
// Created by NBT22 on 2/15/25.
//

#pragma once

namespace luna::core
{
inline void Instance::destroy()
{
	const VkDevice logicalDevice = device_.logicalDevice();
	vkDeviceWaitIdle(logicalDevice);


	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		vkDestroyFramebuffer(logicalDevice, swapChain.framebuffers.at(i), nullptr);
		vkDestroyImageView(logicalDevice, swapChain.imageViews.at(i), nullptr);
	}
	vkDestroySwapchainKHR(logicalDevice, swapChain.swapChain, nullptr);

	for (const VkSampler sampler: samplers_)
	{
		vkDestroySampler(logicalDevice, sampler, nullptr);
	}
	for (Image image: images_)
	{
		image.destroy();
	}

	for (GraphicsPipeline pipeline: graphicsPipelines_)
	{
		pipeline.destroy();
	}
	for (RenderPass renderPass: renderPasses_)
	{
		renderPass.destroy();
	}

	for (const VkDescriptorPool descriptorPool: descriptorPools_)
	{
		vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
	}
	for (DescriptorSetLayout descriptorSetLayout: descriptorSetLayouts_)
	{
		descriptorSetLayout.destroy();
	}

	for (Buffer buffer: buffers_)
	{
		buffer.destroy();
	}

	device_.destroy();
	vkDestroySurfaceKHR(instance_, surface_, nullptr);
	vkDestroyInstance(instance_, nullptr);


	swapChain.images.clear();
	swapChain.images.shrink_to_fit();
	swapChain.imageViews.clear();
	swapChain.imageViews.shrink_to_fit();
	swapChain.framebuffers.clear();
	swapChain.framebuffers.shrink_to_fit();

	samplerIndices_.clear();
	samplers_.clear();
	samplers_.shrink_to_fit();
	imageIndices_.clear();
	images_.clear();
	images_.shrink_to_fit();

	graphicsPipelineIndices_.clear();
	graphicsPipelines_.clear();
	graphicsPipelines_.shrink_to_fit();
	renderPassIndices_.clear();
	renderPasses_.clear();
	renderPasses_.shrink_to_fit();

	descriptorPoolIndices_.clear();
	descriptorSetLayoutIndices_.clear();
	descriptorSetIndices_.clear();
	descriptorPools_.clear();
	descriptorPools_.shrink_to_fit();
	descriptorSetLayouts_.clear();
	descriptorSetLayouts_.shrink_to_fit();
	descriptorSets_.clear();
	descriptorSets_.shrink_to_fit();

	bufferRegionIndices_.clear();
	buffers_.clear();
	buffers_.shrink_to_fit();
}

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
inline void Instance::createRenderPass(const LunaRenderPassCreationInfo *creationInfo, LunaRenderPass *renderPass)
{
	const std::vector<RenderPass>::iterator &renderPassIterator = std::find_if(renderPasses_.begin(),
																			   renderPasses_.end(),
																			   RenderPass::isDestroyed);

	renderPassIndices_.emplace_back(renderPassIterator - renderPasses_.begin());
	renderPasses_.emplace(renderPassIterator, creationInfo, nullptr, &renderPassIndices_.back());
	if (renderPass != nullptr)
	{
		*renderPass = &renderPassIndices_.back();
	}
}
inline void Instance::createRenderPass(const LunaRenderPassCreationInfo2 *creationInfo2, LunaRenderPass *renderPass)
{
	const std::vector<RenderPass>::iterator &renderPassIterator = std::find_if(renderPasses_.begin(),
																			   renderPasses_.end(),
																			   RenderPass::isDestroyed);

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
	renderPassIndices_.emplace_back(renderPassIterator - renderPasses_.begin());
	renderPasses_.emplace(renderPassIterator, &creationInfo, creationInfo2, &renderPassIndices_.back());
	if (renderPass != nullptr)
	{
		*renderPass = &renderPassIndices_.back();
	}
}
inline void Instance::createDescriptorPool(const LunaDescriptorPoolCreationInfo &creationInfo,
										   LunaDescriptorPool *descriptorPool)
{
	descriptorPools_.reserve(descriptorPools_.size() + 1);
	const std::vector<VkDescriptorPool>::iterator poolIterator = std::find(descriptorPools_.begin(),
																		   descriptorPools_.end(),
																		   VK_NULL_HANDLE);
	descriptorPools_.emplace(poolIterator);
	const VkDescriptorPoolCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = creationInfo.flags,
		.maxSets = creationInfo.maxSets,
		.poolSizeCount = creationInfo.poolSizeCount,
		.pPoolSizes = creationInfo.poolSizes,
	};
	vkCreateDescriptorPool(device_.logicalDevice(), &createInfo, nullptr, poolIterator.base());
	descriptorPoolIndices_.emplace_back(poolIterator - descriptorPools_.begin());
	if (descriptorPool != nullptr)
	{
		*descriptorPool = &descriptorPoolIndices_.back();
	}
}
inline void Instance::createDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo &creationInfo,
												LunaDescriptorSetLayout *descriptorSetLayout)
{
	const std::vector<DescriptorSetLayout>::iterator &layoutIterator = std::find_if(descriptorSetLayouts_.begin(),
																					descriptorSetLayouts_.end(),
																					DescriptorSetLayout::isDestroyed);
	descriptorSetLayoutIndices_.emplace_back(layoutIterator - descriptorSetLayouts_.begin());
	descriptorSetLayouts_.emplace(layoutIterator, creationInfo);
	if (descriptorSetLayout != nullptr)
	{
		*descriptorSetLayout = &descriptorSetLayoutIndices_.back();
	}
}
inline void Instance::allocateDescriptorSets(const LunaDescriptorSetAllocationInfo &allocationInfo,
											 LunaDescriptorSet *descriptorSets)
{
	uint32_t slotsFound = 0;
	std::vector<VkDescriptorSetLayout> layouts;
	layouts.reserve(allocationInfo.descriptorSetCount);
	const auto *poolIndex = static_cast<const DescriptorPoolIndex *>(allocationInfo.descriptorPool);
	for (uint32_t i = 0; i < allocationInfo.descriptorSetCount; i++)
	{
		const VkDescriptorSetLayout layout = descriptorSetLayout(allocationInfo.setLayouts[i]).layout();
		const auto *layoutIndex = static_cast<const DescriptorSetLayoutIndex *>(allocationInfo.setLayouts[i]);
		layouts.emplace_back(layout);

		if (slotsFound == i)
		{
			const std::vector<VkDescriptorSet>::iterator descriptorSetIterator = std::find(descriptorSets_.begin(),
																						   descriptorSets_.end(),
																						   VK_NULL_HANDLE);
			if (descriptorSetIterator != descriptorSets_.end())
			{
				descriptorSetIndices_.emplace_back(descriptorSetIterator - descriptorSets_.begin(),
												   layoutIndex,
												   poolIndex);
				descriptorSets[i] = &descriptorSetIndices_.back();

				const VkDescriptorSetAllocateInfo allocateInfo = {
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.descriptorPool = descriptorPool(allocationInfo.descriptorPool),
					.descriptorSetCount = 1,
					.pSetLayouts = &layout,
				};
				vkAllocateDescriptorSets(device_.logicalDevice(), &allocateInfo, descriptorSetIterator.base());
				slotsFound++;
				continue;
			}
		}
		descriptorSetIndices_.emplace_back(descriptorSets_.size() + i - slotsFound, layoutIndex, poolIndex);
		descriptorSets[i] = &descriptorSetIndices_.back();
	}
	const VkDescriptorSetAllocateInfo allocateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool(allocationInfo.descriptorPool),
		.descriptorSetCount = allocationInfo.descriptorSetCount - slotsFound,
		.pSetLayouts = layouts.data() + slotsFound,
	};
	const size_t oldSize = descriptorSets_.size();
	descriptorSets_.resize(oldSize + allocationInfo.descriptorSetCount - slotsFound);
	vkAllocateDescriptorSets(device_.logicalDevice(), &allocateInfo, descriptorSets_.data() + oldSize);
}
inline void Instance::createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo,
											 LunaGraphicsPipeline *pipeline)
{
	const std::vector<GraphicsPipeline>::iterator &pipelineIterator = std::find_if(graphicsPipelines_.begin(),
																				   graphicsPipelines_.end(),
																				   GraphicsPipeline::isDestroyed);
	graphicsPipelineIndices_.emplace_back(pipelineIterator - graphicsPipelines_.begin());
	graphicsPipelines_.emplace(pipelineIterator, creationInfo);
	if (pipeline != nullptr)
	{
		*pipeline = &graphicsPipelineIndices_.back();
	}
}
inline std::vector<Buffer>::iterator Instance::allocateBuffer(const LunaBufferCreationInfo &creationInfo)
{
	buffers_.reserve(buffers_.size() + 1);
	const std::vector<Buffer>::iterator &bufferIterator = std::find_if(buffers_.begin(),
																	   buffers_.end(),
																	   Buffer::isDestroyed);
	const VkBufferCreateInfo bufferCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = creationInfo.flags,
		.size = creationInfo.size,
		.usage = creationInfo.usage,
		.sharingMode = device().sharingMode(),
		.queueFamilyIndexCount = device().familyCount(),
		.pQueueFamilyIndices = device().queueFamilyIndices(),
	};
	buffers_.emplace(bufferIterator, bufferCreateInfo);
	return bufferIterator;
}
inline void Instance::createStagingBuffer(const size_t size)
{
	const LunaBufferCreationInfo bufferCreationInfo = {
		.size = size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	};
	buffer::BufferRegion::createBuffer(bufferCreationInfo, &stagingBuffer_);
}
inline void Instance::copyToStagingBuffer(const uint8_t *data, const size_t size) const
{
	bufferRegion(stagingBuffer_).copyToBuffer(data, size);
}
inline void Instance::createSampler(const LunaSamplerCreationInfo &creationInfo, LunaSampler *sampler)
{
	samplers_.reserve(samplers_.size() + 1);
	const std::vector<VkSampler>::iterator samplerIterator = std::find(samplers_.begin(),
																	   samplers_.end(),
																	   VK_NULL_HANDLE);
	samplerIndices_.emplace_back(samplerIterator - samplers_.begin());
	samplers_.emplace(samplerIterator);
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
	vkCreateSampler(device_.logicalDevice(), &createInfo, nullptr, samplerIterator.base());
	if (sampler != nullptr)
	{
		*sampler = &samplerIndices_.back();
	}
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
	if (pool != nullptr)
	{
		*pool = descriptorPool(descriptorSetIndex.poolIndex);
	}
	if (layout != nullptr)
	{
		*layout = descriptorSetLayout(descriptorSetIndex.layoutIndex);
	}
	if (descriptorSet != nullptr)
	{
		*descriptorSet = descriptorSets_.at(descriptorSetIndex.index);
	}
}
inline Buffer &Instance::buffer(const uint32_t index)
{
	return buffers_.at(index);
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
