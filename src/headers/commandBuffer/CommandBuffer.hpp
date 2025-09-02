//
// Created by NBT22 on 3/20/25.
//

#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Fence.hpp"
#include "Semaphore.hpp"

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

#pragma region "Implmentation"

#include <cassert>
#include "Luna.hpp"

namespace luna::core::commandBuffer
{
inline CommandBuffer::CommandBuffer(const VkDevice logicalDevice,
                                    const VkCommandPool commandPool,
                                    const VkCommandBufferLevel commandBufferLevel,
                                    const void *allocateInfoPNext):
    CommandBuffer(logicalDevice, commandPool, commandBufferLevel, allocateInfoPNext, nullptr)
{}
inline CommandBuffer::CommandBuffer(const VkDevice logicalDevice,
                                    const VkCommandPool commandPool,
                                    const VkCommandBufferLevel commandBufferLevel,
                                    const void *allocateInfoPNext,
                                    const VkSemaphoreCreateInfo *semaphoreCreateInfo):
    semaphore_(logicalDevice, semaphoreCreateInfo)
{
    const VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = allocateInfoPNext,
        .commandPool = commandPool,
        .level = commandBufferLevel,
        .commandBufferCount = 1,
    };
    CHECK_RESULT_THROW(vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffer_));

    constexpr VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    CHECK_RESULT_THROW(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence_));
}

inline CommandBuffer::operator const VkCommandBuffer &() const
{
    return commandBuffer_;
}
inline const VkCommandBuffer *CommandBuffer::operator&() const
{
    return &commandBuffer_;
}

inline void CommandBuffer::destroy(const VkDevice logicalDevice) const
{
    assert(!isRecording_);
    fence_.destroy(logicalDevice);
    semaphore_.destroy(logicalDevice);
}
inline void CommandBuffer::destroy(const VkDevice logicalDevice, const VkCommandPool commandPool)
{
    assert(!isRecording_);
    fence_.destroy(logicalDevice);
    semaphore_.destroy(logicalDevice);
    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer_);
    commandBuffer_ = VK_NULL_HANDLE;
}

inline VkResult CommandBuffer::beginSingleUseCommandBuffer()
{
    assert(!isRecording_);
    CHECK_RESULT_RETURN(vkResetCommandBuffer(commandBuffer_, 0));

    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    CHECK_RESULT_RETURN(vkBeginCommandBuffer(commandBuffer_, &commandBufferBeginInfo));
    isRecording_ = true;
    return VK_SUCCESS;
}
inline VkResult CommandBuffer::submitCommandBuffer(const VkQueue queue,
                                                   const VkSubmitInfo &submitInfo,
                                                   const VkPipelineStageFlags stageMask)
{
    CHECK_RESULT_RETURN(vkEndCommandBuffer(commandBuffer_));
    CHECK_RESULT_RETURN(vkQueueSubmit(queue, 1, &submitInfo, fence_));
    fence_.setWillBeSignaled(true);
    isRecording_ = false;
    if (submitInfo.signalSemaphoreCount > 0)
    {
        for (uint32_t i = 0; i < submitInfo.signalSemaphoreCount; i++)
        {
            if (submitInfo.pSignalSemaphores[i] == semaphore_)
            {
                semaphore_.setIsSignaled(true);
                semaphore_.setStageMask(stageMask);
                break;
            }
        }
    }
    return VK_SUCCESS;
}
inline bool CommandBuffer::getAndSetIsSignaled(const bool value)
{
    const bool oldValue = semaphore_.isSignaled();
    semaphore_.setIsSignaled(value);
    return oldValue;
}
inline VkResult CommandBuffer::waitForFence(const VkDevice logicalDevice, const uint64_t timeout = UINT64_MAX) const
{
    if (!fence_.willBeSignaled())
    {
        return VK_SUCCESS;
    }
    // TODO: If this fails with the default timeout it will block the the render thread for 585 years,
    //  which is unacceptable. While it is not the responsibility of this method to handle this problem,
    //  all usages of this method currently use the default timeout.
    return vkWaitForFences(logicalDevice, 1, &fence_, VK_TRUE, timeout);
}
inline VkResult CommandBuffer::resetFence(const VkDevice logicalDevice)
{
    fence_.setWillBeSignaled(false);
    return vkResetFences(logicalDevice, 1, &fence_);
}

inline bool CommandBuffer::isRecording() const
{
    return isRecording_;
}
inline const Semaphore &CommandBuffer::semaphore() const
{
    return semaphore_;
}
} // namespace luna::core::commandBuffer

#pragma endregion "Implmentation"
