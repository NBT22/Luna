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
inline const RenderPassIndex *Instance::createRenderPass(const LunaRenderPassCreationInfo &creationInfo)
{
	renderPassIndices_.emplace_back(renderPasses_.size());
	if (creationInfo.uniqueName != nullptr)
	{
		renderPassMap_[creationInfo.uniqueName] = renderPasses_.size();
	}
	renderPasses_.emplace_back(creationInfo, &renderPassIndices_.back());
	return &renderPassIndices_.back();
}
inline const RenderPassIndex *Instance::createRenderPass(const LunaRenderPassCreationInfo2 &creationInfo)
{
	renderPassIndices_.emplace_back(renderPasses_.size());
	if (creationInfo.uniqueName != nullptr)
	{
		renderPassMap_[creationInfo.uniqueName] = renderPasses_.size();
	}
	renderPasses_.emplace_back(creationInfo, &renderPassIndices_.back());
	return &renderPassIndices_.back();
}
inline const GraphicsPipelineIndex *Instance::createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo
																			 &creationInfo)
{
	const RenderPassIndex *index = static_cast<const RenderPassSubpassIndex *>(creationInfo.subpass)->renderPassIndex;
	renderPass(index).pipelineIndices.emplace_back(graphicsPipelines_.size());

	graphicsPipelineIndices_.emplace_back(graphicsPipelines_.size());
	if (creationInfo.uniqueName != nullptr)
	{
		graphicsPipelineMap_[creationInfo.uniqueName] = graphicsPipelines_.size();
	}
	graphicsPipelines_.emplace_back(creationInfo);
	return &graphicsPipelineIndices_.back();
}
inline uint32_t Instance::allocateBuffer(const LunaBufferCreationInfo &creationInfo)
{
	VkBufferCreateInfo bufferCreateInfo = {
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
inline const SwapChain &Instance::swapChain() const
{
	return swapChain_;
}
inline SwapChain &Instance::swapChain()
{
	return swapChain_;
}
inline RenderPass &Instance::renderPass(const LunaRenderPass index)
{
	return renderPasses_.at(static_cast<const RenderPassIndex *>(index)->index);
}
inline GraphicsPipeline &Instance::graphicsPipeline(const uint32_t index)
{
	return graphicsPipelines_.at(index);
}
} // namespace luna::core
