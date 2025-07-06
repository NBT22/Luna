//
// Created by NBT22 on 5/29/25.
//

#pragma once

#include <cassert>

namespace luna::core::commandBuffer
{
template<uint32_t count> CommandBufferArray<count>::CommandBufferArray(const VkDevice logicalDevice,
                                                                       const VkCommandPool commandPool,
                                                                       const VkCommandBufferLevel commandBufferLevel,
                                                                       const void *allocateInfoPNext)
{
    const VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = allocateInfoPNext,
        .commandPool = commandPool,
        .level = commandBufferLevel,
        .commandBufferCount = count,
    };
    CHECK_RESULT_THROW(vkAllocateCommandBuffers(logicalDevice, &allocateInfo, commandBuffers_.data()));

    constexpr VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (uint32_t i = 0; i < count; i++)
    {
        CHECK_RESULT_THROW(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fences_.at(i)));
    }
}
template<uint32_t count>
CommandBufferArray<count>::CommandBufferArray(const VkDevice logicalDevice,
                                              const VkCommandPool commandPool,
                                              const VkCommandBufferLevel commandBufferLevel,
                                              const void *allocateInfoPNext,
                                              const VkSemaphoreCreateInfo *semaphoreCreateInfo):
    CommandBufferArray(logicalDevice, commandPool, commandBufferLevel, allocateInfoPNext)
{
    for (Semaphore &semaphore: semaphores_)
    {
        semaphore = Semaphore(logicalDevice, semaphoreCreateInfo);
    }
}

template<uint32_t count> CommandBufferArray<count>::operator const VkCommandBuffer &() const
{
    return commandBuffers_.at(index_);
}
template<uint32_t count> const VkCommandBuffer *CommandBufferArray<count>::operator&() const
{
    return &commandBuffers_.at(index_);
}

template<uint32_t count> void CommandBufferArray<count>::destroy(const VkDevice logicalDevice) const
{
    for (uint32_t i = 0; i < count; i++)
    {
        assert(!isRecordings_[i]);
        vkDestroyFence(logicalDevice, fences_[i], nullptr);
        semaphores_[i].destroy(logicalDevice);
    }
}

template<uint32_t count> VkResult CommandBufferArray<count>::beginSingleUseCommandBuffer()
{
    assert(!isRecordings_.at(index_));
    CHECK_RESULT_RETURN(vkResetCommandBuffer(commandBuffers_.at(index_), 0));

    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    CHECK_RESULT_RETURN(vkBeginCommandBuffer(commandBuffers_.at(index_), &commandBufferBeginInfo));
    isRecordings_.at(index_) = true;
    return VK_SUCCESS;
}
template<uint32_t count> VkResult CommandBufferArray<count>::submitCommandBuffer(const VkQueue queue,
                                                                                 const VkSubmitInfo &submitInfo,
                                                                                 const VkPipelineStageFlags stageMask)
{
    CHECK_RESULT_RETURN(vkEndCommandBuffer(commandBuffers_.at(index_)));
    CHECK_RESULT_RETURN(vkQueueSubmit(queue, 1, &submitInfo, fences_.at(index_)));
    fences_.at(index_).setWillBeSignaled(true);
    isRecordings_.at(index_) = false;
    if (submitInfo.signalSemaphoreCount > 0)
    {
        for (uint32_t i = 0; i < submitInfo.signalSemaphoreCount; i++)
        {
            if (submitInfo.pSignalSemaphores[i] == semaphores_.at(index_))
            {
                semaphores_.at(index_).setIsSignaled(true);
                semaphores_.at(index_).setStageMask(stageMask);
                break;
            }
        }
    }
    index_ = (index_ + 1) % count;
    return VK_SUCCESS;
}
template<uint32_t count> bool CommandBufferArray<count>::getAndSetIsSignaled(const bool value)
{
    const bool oldValue = semaphores_.at(index_).isSignaled();
    semaphores_.at(index_).setIsSignaled(value);
    return oldValue;
}
template<uint32_t count> VkResult CommandBufferArray<count>::waitForAllFences(const VkDevice logicalDevice,
                                                                              const uint64_t timeout) const
{
    std::vector<VkFence> fences;
    fences.reserve(count);
    uint32_t waitCount = 0;
    for (const Fence &fence: fences_)
    {
        if (fence.willBeSignaled())
        {
            fences.emplace_back(fence);
            waitCount++;
        }
    }
    if (waitCount == 0)
    {
        return VK_SUCCESS;
    }
    return vkWaitForFences(logicalDevice, waitCount, fences.data(), VK_TRUE, timeout);
}
template<uint32_t count> VkResult CommandBufferArray<count>::waitForFence(const VkDevice logicalDevice,
                                                                          const uint64_t timeout) const
{
    if (!fences_.at(index_).willBeSignaled())
    {
        return VK_SUCCESS;
    }
    // TODO: If this fails with the default timeout it will block the the render thread for 585 years,
    //  which is unacceptable. While it is not the responsibility of this method to handle this problem,
    //  all usages of this method currently use the default timeout.
    return vkWaitForFences(logicalDevice, 1, &fences_.at(index_), VK_TRUE, timeout);
}
template<uint32_t count> VkResult CommandBufferArray<count>::resetFence(const VkDevice logicalDevice)
{
    fences_.at(index_).setWillBeSignaled(false);
    return vkResetFences(logicalDevice, 1, &fences_.at(index_));
}
template<uint32_t count> VkResult CommandBufferArray<count>::recreateSemaphores(const VkDevice logicalDevice)
{
    // TODO: Should this function take an take an array of creation infos instead of doing this?
    constexpr VkSemaphoreCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    for (Semaphore &semaphore : semaphores_)
    {
        semaphore.destroy(logicalDevice);
        CHECK_RESULT_RETURN(semaphore.recreate(logicalDevice, &createInfo));
    }
    return VK_SUCCESS;
}

template<uint32_t count> bool CommandBufferArray<count>::isRecording() const
{
    return isRecordings_.at(index_);
}
template<uint32_t count> const Semaphore &CommandBufferArray<count>::semaphore() const
{
    assert(!semaphores_.at(index_).isSignaled());
    return semaphores_.at(index_);
}
} // namespace luna::core::commandBuffer
