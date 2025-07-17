//
// Created by NBT22 on 5/29/25.
//

#pragma once

#include <luna/core/Fence.hpp>
#include <luna/core/Semaphore.hpp>

namespace luna::core::commandBuffer
{
class CommandBufferArray
{
    public:
        CommandBufferArray() = default;
        // CommandBufferArray(CommandBufferArray&&) {}
        CommandBufferArray(VkDevice logicalDevice,
                           VkCommandPool commandPool,
                           VkCommandBufferLevel commandBufferLevel,
                           const void *allocateInfoPNext,
                           uint32_t count);
        CommandBufferArray(VkDevice logicalDevice,
                           VkCommandPool commandPool,
                           VkCommandBufferLevel commandBufferLevel,
                           const void *allocateInfoPNext,
                           const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                           uint32_t count);

        // constexpr CommandBufferArray& operator=(const CommandBufferArray&) {return *this;}

        operator const VkCommandBuffer &() const;
        const VkCommandBuffer *operator&() const;

        void destroy(VkDevice logicalDevice) const;

        VkResult beginSingleUseCommandBuffer();
        VkResult submitCommandBuffer(VkQueue queue,
                                     const VkSubmitInfo &submitInfo,
                                     VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        bool getAndSetIsSignaled(bool value);
        VkResult waitForAllFences(VkDevice logicalDevice, uint64_t timeout = UINT64_MAX) const;
        VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout = UINT64_MAX) const;
        VkResult resetFence(VkDevice logicalDevice);
        VkResult recreateSemaphores(VkDevice logicalDevice);

        [[nodiscard]] bool isRecording() const;
        [[nodiscard]] const Semaphore &semaphore() const;

    private:
        uint32_t index_{};
        std::vector<bool> isRecordings_{};
        std::vector<VkCommandBuffer> commandBuffers_{};
        std::vector<Fence> fences_{};
        std::vector<Semaphore> semaphores_{};
};
} // namespace luna::core::commandBuffer

#include <luna/implementations/core/CommandBuffer/CommandBufferArray.ipp>
