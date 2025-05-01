//
// Created by NBT22 on 5/1/25.
//

#pragma once

namespace luna::core
{
inline CommandPool::CommandPool()
{
    assert(isDestroyed_);
    isDestroyed_ = false;
}

inline void CommandPool::destroy(const VkDevice logicalDevice)
{
    if (isDestroyed_)
    {
        return;
    }
    for (const CommandBuffer &commandBuffer: commandBuffers_)
    {
        assert(commandBuffer.isRecording_);
        vkDestroyFence(logicalDevice, commandBuffer.fence_, nullptr);
    }
    vkDestroyCommandPool(logicalDevice, commandPool_, nullptr);
    isDestroyed_ = true;
}

inline VkResult CommandPool::allocate(const VkDevice logicalDevice, const VkCommandPoolCreateInfo &poolCreateInfo)
{
    CHECK_RESULT_RETURN(vkCreateCommandPool(logicalDevice, &poolCreateInfo, nullptr, &commandPool_));
    return VK_SUCCESS;
}
inline VkResult CommandPool::allocateCommandBuffer(VkDevice logicalDevice,
                                                   VkCommandBufferLevel commandBufferLevel,
                                                   const void *allocateInfoPNext)
{
    TRY_CATCH_RESULT(commandBuffers_.emplace_back(logicalDevice, commandPool_, commandBufferLevel, allocateInfoPNext));
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
