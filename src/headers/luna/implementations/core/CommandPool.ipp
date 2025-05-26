//
// Created by NBT22 on 5/1/25.
//

#pragma once

namespace luna::core
{
inline bool CommandPool::isDestroyed(const CommandPool &commandPool)
{
    return commandPool.isDestroyed_;
}

inline CommandPool::CommandPool(const VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo)
{
    CHECK_RESULT_THROW(allocate(logicalDevice, creationInfo));
}

inline CommandPool::operator const VkCommandPool &() const
{
    return commandPool_;
}

inline VkResult CommandPool::allocate(const VkDevice logicalDevice, const VkCommandPoolCreateInfo &poolCreateInfo)
{
    isDestroyed_ = false;
    CHECK_RESULT_RETURN(vkCreateCommandPool(logicalDevice, &poolCreateInfo, nullptr, &commandPool_));
    return VK_SUCCESS;
}
inline VkResult CommandPool::allocate(const VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo)
{
    const VkCommandPoolCreateInfo poolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = creationInfo.flags,
        // TODO: queueFamilyIndex
    };
    CHECK_RESULT_RETURN(allocate(logicalDevice, poolCreateInfo));
    return VK_SUCCESS;
}
inline VkResult CommandPool::allocateCommandBuffer(VkDevice logicalDevice,
                                                   VkCommandBufferLevel commandBufferLevel,
                                                   const void *allocateInfoPNext)
{
    assert(!isDestroyed_);
    TRY_CATCH_RESULT(commandBuffers_.emplace_back(logicalDevice, commandPool_, commandBufferLevel, allocateInfoPNext));
    return VK_SUCCESS;
}
inline VkResult CommandPool::allocateCommandBuffer(VkDevice logicalDevice,
                                                   VkCommandBufferLevel commandBufferLevel,
                                                   const void *allocateInfoPNext,
                                                   const VkSemaphoreCreateInfo *semaphoreCreateInfo)
{
    assert(!isDestroyed_);
    TRY_CATCH_RESULT(commandBuffers_.emplace_back(logicalDevice,
                                                  commandPool_,
                                                  commandBufferLevel,
                                                  allocateInfoPNext,
                                                  semaphoreCreateInfo));
    return VK_SUCCESS;
}
inline VkResult CommandPool::reset(const VkDevice logicalDevice,
                                   const VkCommandPoolResetFlagBits flags,
                                   const uint64_t timeout = UINT64_MAX)
{
    std::vector<VkFence> fences;
    fences.reserve(commandBuffers_.size());
    for (CommandBuffer &commandBuffer: commandBuffers_)
    {
        // TODO: This is cursed and could cause bugs.
        //  Fences should be reworked so that I can check if the fence needs to be waited upon.
        if (commandBuffer.semaphore_.isSignaled())
        {
            fences.emplace_back(commandBuffer.fence_);
        }
        commandBuffer.isRecording_ = false;
    }
    // TODO: If this fails with the default timeout it will block the the render thread for 585 years,
    //  which is unacceptable. While it is not the responsibility of this method to handle this problem,
    //  all usages of this method currently use the default timeout.
    CHECK_RESULT_RETURN(vkWaitForFences(logicalDevice, fences.size(), fences.data(), VK_TRUE, timeout));
    CHECK_RESULT_RETURN(vkResetCommandPool(logicalDevice, commandPool_, flags));

    return VK_SUCCESS;
}

inline const CommandBuffer &CommandPool::commandBuffer(const uint32_t index) const
{
    return commandBuffers_.at(index);
}
inline CommandBuffer &CommandPool::commandBuffer(const uint32_t index)
{
    return commandBuffers_.at(index);
}
} // namespace luna::core
