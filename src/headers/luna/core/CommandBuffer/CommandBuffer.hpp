//
// Created by NBT22 on 3/20/25.
//

#pragma once

#include <luna/core/Semaphore.hpp>
#include <luna/core/CommandBuffer.hpp>

namespace luna::core::commandBuffer
{
template<uint32_t arraySize> class CommandPool;

class CommandBuffer: public core::CommandBuffer
{
    public:
        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext);
        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext,
                      const VkSemaphoreCreateInfo *semaphoreCreateInfo);

        operator const VkCommandBuffer &() const override;
        const VkCommandBuffer *operator&() const override;

        void destroy(VkDevice logicalDevice) const override;

        VkResult beginSingleUseCommandBuffer() override;
        VkResult submitCommandBuffer(VkQueue queue,
                                     const VkSubmitInfo &submitInfo,
                                     VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) override;
        bool getAndSetIsSignaled(bool value) override;
        VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout) const override;
        VkResult resetFence(VkDevice logicalDevice) const override;

        [[nodiscard]] bool isRecording() const override;
        [[nodiscard]] const Semaphore &semaphore() const override;

    private:
        bool isRecording_{};
        VkCommandBuffer commandBuffer_{};
        VkFence fence_{};
        Semaphore semaphore_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandBuffer/CommandBuffer.ipp>
