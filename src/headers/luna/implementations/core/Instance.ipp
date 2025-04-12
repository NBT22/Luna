//
// Created by NBT22 on 2/15/25.
//

#pragma once

namespace luna::core
{
inline const RenderPass &renderPass(const LunaRenderPass renderPass)
{
	return renderPasses.at(static_cast<const RenderPassIndex *>(renderPass)->index);
}
inline VkDescriptorPool descriptorPool(const LunaDescriptorPool descriptorPool)
{
	return descriptorPools.at(static_cast<const DescriptorPoolIndex *>(descriptorPool)->index);
}
inline const DescriptorSetLayout &descriptorSetLayout(const LunaDescriptorSetLayout layout)
{
	return descriptorSetLayouts.at(static_cast<const DescriptorSetLayoutIndex *>(layout)->index);
}
inline VkDescriptorSet descriptorSet(const LunaDescriptorSet descriptorSet)
{
	return descriptorSets.at(static_cast<const DescriptorSetIndex *>(descriptorSet)->index);
}
inline void descriptorSet(const LunaDescriptorSet index,
						  VkDescriptorPool *pool,
						  DescriptorSetLayout *layout,
						  VkDescriptorSet *descriptorSet)
{
	const DescriptorSetIndex *descriptorSetIndex = static_cast<const DescriptorSetIndex *>(index);
	if (pool != nullptr)
	{
		*pool = descriptorPools.at(descriptorSetIndex->poolIndex->index);
	}
	if (layout != nullptr)
	{
		*layout = descriptorSetLayout(descriptorSetIndex->layoutIndex);
	}
	if (descriptorSet != nullptr)
	{
		*descriptorSet = descriptorSets.at(descriptorSetIndex->index);
	}
}
inline const buffer::BufferRegion &bufferRegion(const LunaBuffer buffer)
{
	const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(buffer);
	return buffers.at(index->bufferIndex).region(index->bufferRegionIndex);
}
inline const buffer::BufferRegion &bufferRegion(const buffer::BufferRegionIndex index)
{
	return buffers.at(index.bufferIndex).region(index.bufferRegionIndex);
}
inline VkBuffer stagingBuffer()
{
	const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(stagingBufferIndex);
	return buffers.at(index->bufferIndex).buffer();
}
inline size_t stagingBufferOffset()
{
	if (stagingBufferIndex == nullptr)
	{
		return -1ull;
	}
	return bufferRegion(stagingBufferIndex).offset();
}
inline VkSampler sampler(const LunaSampler sampler)
{
	return samplers.at(static_cast<const SamplerIndex *>(sampler)->index);
}
} // namespace luna::core
