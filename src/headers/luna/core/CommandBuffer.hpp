//
// Created by NBT22 on 6/2/25.
//

#pragma once

#include <luna/core/commandBuffer/CommandBuffer.hpp>
#include <luna/core/commandBuffer/CommandBufferArray.hpp>
#include <string>

namespace luna::core
{
class CommandBuffer
{
    public:
        enum class Type : uint8_t
        {
            SINGLE,
            ARRAY,
        };

        CommandBuffer(const CommandBuffer &other);
        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext,
                      uint32_t arraySize);
        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext,
                      const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                      uint32_t arraySize);

        operator const VkCommandBuffer &() const;
        const VkCommandBuffer *operator&() const;

        void destroy(VkDevice logicalDevice) const;

        VkResult resizeArray(VkDevice logicalDevice,
                             VkCommandPool commandPool,
                             VkCommandBufferLevel commandBufferLevel,
                             const void *allocateInfoPNext,
                             const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                             uint32_t arraySize,
                             uint64_t timeout = UINT64_MAX);
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

        [[nodiscard]] Type type() const;
        [[nodiscard]] std::string typeAsString() const;
        [[nodiscard]] const commandBuffer::CommandBuffer &commandBuffer() const;
        [[nodiscard]] const commandBuffer::CommandBufferArray &commandBufferArray() const;

    private:
        Type type_{};
        commandBuffer::CommandBuffer commandBuffer_{};
        commandBuffer::CommandBufferArray commandBufferArray_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandBuffer.ipp>
