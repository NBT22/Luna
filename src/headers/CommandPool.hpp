//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <cstdint>
#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"

namespace luna
{
class CommandPool
{
    public:
        static bool isDestroyed(const CommandPool &commandPool);

        operator const VkCommandPool &() const;

        CommandPool() = default;
        CommandPool(VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo);

        void destroy(VkDevice logicalDevice);

        VkResult allocate(VkDevice logicalDevice, const VkCommandPoolCreateInfo &poolCreateInfo);
        VkResult allocate(VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo);
        VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext,
                                       uint32_t arraySize = 1);
        VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext,
                                       const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                                       uint32_t arraySize = 1);
        VkResult reset(VkDevice logicalDevice, VkCommandPoolResetFlagBits flags, uint64_t timeout = UINT64_MAX) const;

        [[nodiscard]] const CommandBuffer &commandBuffer(uint32_t index = 0) const;
        [[nodiscard]] CommandBuffer &commandBuffer(uint32_t index = 0);

    private:
        bool isDestroyed_{true};
        VkCommandPool commandPool_{};
        std::vector<CommandBuffer> commandBuffers_{};
};
} // namespace luna

#pragma region "Implmentation"

#include <cassert>
#include <volk.h>
#include "Luna.hpp"

namespace luna
{
inline bool CommandPool::isDestroyed(const CommandPool &commandPool)
{
    return commandPool.isDestroyed_;
}

inline CommandPool::CommandPool(const VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo)
{
    CHECK_RESULT_THROW(allocate(logicalDevice, creationInfo));
}

inline void CommandPool::destroy(const VkDevice logicalDevice)
{
    if (isDestroyed_)
    {
        return;
    }
    for (const CommandBuffer &commandBuffer: commandBuffers_)
    {
        commandBuffer.destroy(logicalDevice);
    }
    vkDestroyCommandPool(logicalDevice, commandPool_, nullptr);
    isDestroyed_ = true;
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
                                                   const void *allocateInfoPNext,
                                                   const uint32_t arraySize)
{
    assert(!isDestroyed_);
    TRY_CATCH_RESULT(commandBuffers_.emplace_back(logicalDevice,
                                                  commandPool_,
                                                  commandBufferLevel,
                                                  allocateInfoPNext,
                                                  arraySize));
    return VK_SUCCESS;
}
inline VkResult CommandPool::allocateCommandBuffer(VkDevice logicalDevice,
                                                   VkCommandBufferLevel commandBufferLevel,
                                                   const void *allocateInfoPNext,
                                                   const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                                                   const uint32_t arraySize)
{
    assert(!isDestroyed_);
    TRY_CATCH_RESULT(commandBuffers_.emplace_back(logicalDevice,
                                                  commandPool_,
                                                  commandBufferLevel,
                                                  allocateInfoPNext,
                                                  semaphoreCreateInfo,
                                                  arraySize));
    return VK_SUCCESS;
}
inline VkResult CommandPool::reset(const VkDevice logicalDevice,
                                   const VkCommandPoolResetFlagBits flags,
                                   const uint64_t timeout) const
{
    for (const CommandBuffer &commandBuffer: commandBuffers_)
    {
        if (commandBuffer.type() == CommandBuffer::Type::ARRAY)
        {
            CHECK_RESULT_RETURN(commandBuffer.commandBufferArray().waitForAllFences(logicalDevice, timeout));
        } else
        {
            CHECK_RESULT_RETURN(commandBuffer.commandBuffer().waitForFence(logicalDevice, timeout));
        }
    }
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
} // namespace luna

#pragma endregion "Implmentation"
