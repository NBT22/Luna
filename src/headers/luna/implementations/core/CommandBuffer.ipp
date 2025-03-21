//
// Created by NBT22 on 3/20/25.
//

#pragma once

#include <cassert>

namespace luna::core
{
inline void CommandBuffer::allocateCommandBuffer(const VkDevice logicalDevice,
												 const VkCommandPoolCreateInfo &poolCreateInfo,
												 const VkCommandBufferLevel commandBufferLevel,
												 const void *allocateInfoPNext)
{
	vkCreateCommandPool(logicalDevice, &poolCreateInfo, nullptr, &commandPool_);
	const VkCommandBufferAllocateInfo allocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = allocateInfoPNext,
		.commandPool = commandPool_,
		.level = commandBufferLevel,
		.commandBufferCount = 1,
	};
	vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffer_);

	constexpr VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence);
}
inline void CommandBuffer::beginSingleUseCommandBuffer()
{
	assert(!isRecording_);
	vkResetCommandBuffer(commandBuffer_, 0);

	constexpr VkCommandBufferBeginInfo commandBufferBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(commandBuffer_, &commandBufferBeginInfo);
	isRecording_ = true;
}
inline void CommandBuffer::submitCommandBuffer(const VkQueue queue, const VkSubmitInfo &submitInfo)
{
	vkEndCommandBuffer(commandBuffer_);
	vkQueueSubmit(queue, 1, &submitInfo, fence);
	isRecording_ = false;
}
inline void CommandBuffer::setRecording(const bool value)
{
	isRecording_ = value;
}
inline VkResult CommandBuffer::waitForFence(const VkDevice logicalDevice, const uint64_t timeout = UINT64_MAX) const
{
	// TODO: If this fails with the default timeout it will block the the render thread for 585 years,
	//  which is unacceptable. While it is not the responsibility of this method to handle this problem,
	//  all usages of this method currently use the default timeout.
	return vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, timeout);
}

inline const VkCommandBuffer &CommandBuffer::commandBuffer() const
{
	return commandBuffer_;
}
inline bool CommandBuffer::isRecording() const
{
	return isRecording_;
}
} // namespace luna::core
