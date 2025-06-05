//
// Created by NBT22 on 5/29/25.
//

#pragma once

#include <array>
#include <luna/core/CommandBuffer.hpp>
#include <luna/core/Fence.hpp>

namespace luna::core::commandBuffer
{
template<uint32_t count> class CommandBufferArray final: public core::CommandBuffer
{
    public:
        CommandBufferArray(VkDevice logicalDevice,
                           VkCommandPool commandPool,
                           VkCommandBufferLevel commandBufferLevel,
                           const void *allocateInfoPNext);
        CommandBufferArray(VkDevice logicalDevice,
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
        VkResult waitForAllFences(VkDevice logicalDevice, uint64_t timeout = UINT64_MAX) const;
        VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout = UINT64_MAX) const override;
        VkResult resetFence(VkDevice logicalDevice) override;

        [[nodiscard]] bool isRecording() const override;
        [[nodiscard]] const Semaphore &semaphore() const override;

    private:
        uint32_t index_{};
        std::array<bool, count> isRecordings_{};
        std::array<VkCommandBuffer, count> commandBuffers_{};
        std::array<Fence, count> fences_{};
        std::array<Semaphore, count> semaphores_{};
};
} // namespace luna::core::commandBuffer

#include <luna/implementations/core/CommandBuffer/CommandBufferArray.ipp>
