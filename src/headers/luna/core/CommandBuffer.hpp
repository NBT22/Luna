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
		CommandBuffer() = default;

		void allocateCommandBuffer(VkDevice logicalDevice,
								   const VkCommandPoolCreateInfo &poolCreateInfo,
								   VkCommandBufferLevel commandBufferLevel,
								   const void *allocateInfoPNext);
		void beginSingleUseCommandBuffer();
		void submitCommandBuffer(VkQueue queue, const VkSubmitInfo &submitInfo);
		void setRecording(bool value);
		VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout) const;

		[[nodiscard]] const VkCommandBuffer &commandBuffer() const;
		[[nodiscard]] bool isRecording() const;

		VkFence fence{};

	private:
		VkCommandBuffer commandBuffer_{};
		VkCommandPool commandPool_{};
		bool isRecording_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandBuffer.ipp>
