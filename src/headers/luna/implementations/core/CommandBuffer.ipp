//
// Created by NBT22 on 3/20/25.
//

#pragma once

#include <cassert>

namespace luna::helpers
{
static VkResult createCommandBuffer(const VkDevice logicalDevice,
                                    const VkCommandPool commandPool,
                                    const VkCommandBufferLevel commandBufferLevel,
                                    const void *allocateInfoPNext,
                                    VkCommandBuffer *commandBuffer,
                                    VkFence *fence)
{
    const VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = allocateInfoPNext,
        .commandPool = commandPool,
        .level = commandBufferLevel,
        .commandBufferCount = 1,
    };
    CHECK_RESULT_RETURN(vkAllocateCommandBuffers(logicalDevice, &allocateInfo, commandBuffer));

    constexpr VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    CHECK_RESULT_RETURN(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, fence));

    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna::core
{
inline CommandBuffer::CommandBuffer(const VkDevice logicalDevice,
                                    const VkCommandPool commandPool,
                                    const VkCommandBufferLevel commandBufferLevel,
                                    const void *allocateInfoPNext)
{
    CHECK_RESULT_THROW(helpers::createCommandBuffer(logicalDevice,
                                                    commandPool,
                                                    commandBufferLevel,
                                                    allocateInfoPNext,
                                                    &commandBuffer_,
                                                    &fence_));
}
inline CommandBuffer::CommandBuffer(const VkDevice logicalDevice,
                                    const VkCommandPool commandPool,
                                    const VkCommandBufferLevel commandBufferLevel,
                                    const void *allocateInfoPNext,
                                    const VkSemaphoreCreateInfo *semaphoreCreateInfo):
    semaphore_(logicalDevice, semaphoreCreateInfo)
{
    CHECK_RESULT_THROW(helpers::createCommandBuffer(logicalDevice,
                                                    commandPool,
                                                    commandBufferLevel,
                                                    allocateInfoPNext,
                                                    &commandBuffer_,
                                                    &fence_));
}

inline CommandBuffer::operator const VkCommandBuffer &() const
{
    return commandBuffer_;
}
inline const VkCommandBuffer *CommandBuffer::operator&() const
{
    return &commandBuffer_;
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
    // TODO: If this fails with the default timeout it will block the the render thread for 585 years,
    //  which is unacceptable. While it is not the responsibility of this method to handle this problem,
    //  all usages of this method currently use the default timeout.
    return vkWaitForFences(logicalDevice, 1, &fence_, VK_TRUE, timeout);
}
inline VkResult CommandBuffer::resetFence(const VkDevice logicalDevice) const
{
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
} // namespace luna::core
