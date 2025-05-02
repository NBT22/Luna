//
// Created by NBT22 on 3/20/25.
//

#pragma once

#include <luna/core/Semaphore.hpp>

namespace luna::core
{
class CommandBuffer
{
    public:
        friend class CommandPool;

        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext);
        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext,
                      const VkSemaphoreCreateInfo *semaphoreCreateInfo);

        operator const VkCommandBuffer &() const;
        const VkCommandBuffer *operator&() const;

        VkResult beginSingleUseCommandBuffer();
        VkResult submitCommandBuffer(VkQueue queue,
                                     const VkSubmitInfo &submitInfo,
                                     VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        bool getAndSetIsSignaled(bool value);
        VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout) const;
        VkResult resetFence(VkDevice logicalDevice) const;

        [[nodiscard]] bool isRecording() const;
        [[nodiscard]] const Semaphore &semaphore() const;

    private:
        bool isRecording_{};
        VkCommandBuffer commandBuffer_{};
        VkFence fence_{};
        Semaphore semaphore_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandBuffer.ipp>
