//
// Created by NBT22 on 2/15/25.
//

#pragma once

namespace luna::core
{
inline void Instance::addNewDevice(const LunaDeviceCreationInfo2 &creationInfo)
{
	device_ = Device(creationInfo);
}
inline const RenderPassIndex *Instance::createRenderPass(const LunaRenderPassCreationInfo *creationInfo)
{
	renderPassIndices_.emplace_back(renderPasses_.size());
	if (creationInfo->uniqueName != nullptr)
	{
		renderPassMap_[creationInfo->uniqueName] = renderPasses_.size();
	}
	renderPasses_.emplace_back(creationInfo, nullptr, &renderPassIndices_.back());
	return &renderPassIndices_.back();
}
inline const RenderPassIndex *Instance::createRenderPass(const LunaRenderPassCreationInfo2 *creationInfo2)
{
	renderPassIndices_.emplace_back(renderPasses_.size());
	if (creationInfo2->uniqueName != nullptr)
	{
		renderPassMap_[creationInfo2->uniqueName] = renderPasses_.size();
	}
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
		.uniqueName = creationInfo2->uniqueName,
	};
	renderPasses_.emplace_back(&creationInfo, creationInfo2, &renderPassIndices_.back());
	return &renderPassIndices_.back();
}
inline const GraphicsPipelineIndex *Instance::createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo
																			 &creationInfo)
{
	const RenderPassIndex *index = static_cast<const RenderPassSubpassIndex *>(creationInfo.subpass)->renderPassIndex;
	renderPass_(index).pipelineIndices.emplace_back(graphicsPipelines.size());

	graphicsPipelineIndices_.emplace_back(graphicsPipelines.size());
	if (creationInfo.uniqueName != nullptr)
	{
		graphicsPipelineMap_[creationInfo.uniqueName] = graphicsPipelines.size();
	}
	graphicsPipelines.emplace_back(creationInfo);
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
inline VkSurfaceKHR Instance::surface() const
{
	return surface_;
}
inline const RenderPass &Instance::renderPass(const LunaRenderPass index) const
{
	return renderPasses_.at(static_cast<const RenderPassIndex *>(index)->index);
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

inline RenderPass &Instance::renderPass_(const LunaRenderPass index)
{
	return renderPasses_.at(static_cast<const RenderPassIndex *>(index)->index);
}
} // namespace luna::core
