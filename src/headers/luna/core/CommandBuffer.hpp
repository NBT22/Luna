//
// Created by NBT22 on 3/20/25.
//

#pragma once

#include <vulkan/vulkan_core.h>

namespace luna::core
{
class CommandBuffer
{
	public:
		CommandBuffer();

		void destroy(VkDevice logicalDevice);

		VkResult allocateCommandBuffer(VkDevice logicalDevice,
									   const VkCommandPoolCreateInfo &poolCreateInfo,
									   VkCommandBufferLevel commandBufferLevel,
									   const void *allocateInfoPNext);
		VkResult beginSingleUseCommandBuffer();
		VkResult submitCommandBuffer(VkQueue queue, const VkSubmitInfo &submitInfo);
		void setRecording(bool value);
		VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout) const;
		VkResult resetFence(VkDevice logicalDevice) const;

		[[nodiscard]] const VkCommandBuffer &commandBuffer() const;
		[[nodiscard]] bool isRecording() const;

	private:
		bool isDestroyed_{true};
		bool isRecording_{};
		VkCommandBuffer commandBuffer_{};
		// TODO: Having a single command buffer per pool is not really ideal
		VkCommandPool commandPool_{};
		VkFence fence_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandBuffer.ipp>
