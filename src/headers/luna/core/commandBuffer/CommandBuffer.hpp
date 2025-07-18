//
// Created by NBT22 on 3/20/25.
//

#pragma once

#include <luna/core/Fence.hpp>
#include <luna/core/Semaphore.hpp>

namespace luna::core::commandBuffer
{
class CommandBuffer
{
    public:
        CommandBuffer() = default;
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

        void destroy(VkDevice logicalDevice) const;
        void destroy(VkDevice logicalDevice, VkCommandPool commandPool);

        VkResult beginSingleUseCommandBuffer();
        VkResult submitCommandBuffer(VkQueue queue,
                                     const VkSubmitInfo &submitInfo,
                                     VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        bool getAndSetIsSignaled(bool value);
        VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout) const;
        VkResult resetFence(VkDevice logicalDevice);

        [[nodiscard]] bool isRecording() const;
        [[nodiscard]] const Semaphore &semaphore() const;

    private:
        bool isRecording_{};
        VkCommandBuffer commandBuffer_{};
        Fence fence_{};
        Semaphore semaphore_{};
};
} // namespace luna::core::commandBuffer

#include <luna/implementations/core/CommandBuffer/CommandBuffer.ipp>
