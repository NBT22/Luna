//
// Created by NBT22 on 3/28/26.
//

#include <cassert>
#include <cstdint>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaTypes.h>
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"
#include "helpers/Handle.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna
{
CommandBuffer::CommandBuffer(const VkCommandPool commandPool, const VkCommandBufferLevel commandBufferLevel):
    type_(Type::SINGLE),
    commandBuffer_(device, commandPool, commandBufferLevel)
{}
CommandBuffer::CommandBuffer(const VkCommandPool commandPool,
                             const VkCommandBufferLevel commandBufferLevel,
                             const uint32_t arraySize):
    type_(Type::ARRAY),
    commandBufferArray_(device, commandPool, commandBufferLevel, arraySize)
{
    assert(arraySize > 1);
}

VkResult CommandBuffer::resizeArray(const VkCommandPool commandPool,
                                    const VkCommandBufferLevel commandBufferLevel,
                                    const uint32_t arraySize,
                                    const uint64_t timeout)
{
    if ((type_ == Type::SINGLE && arraySize == 1) || (type_ == Type::ARRAY && arraySize == commandBufferArray_.size()))
    {
        return VK_SUCCESS;
    }
    switch (type_)
    {
        case Type::SINGLE:
            CHECK_RESULT_RETURN(commandBuffer_.waitForFence(device, timeout));
            commandBuffer_.destroy(device, commandPool);
            commandBufferArray_ = commandBuffer::CommandBufferArray(device, commandPool, commandBufferLevel, arraySize);
            type_ = Type::ARRAY;
            break;
        case Type::ARRAY:
            assert(!commandBufferArray_.anyRecording());
            CHECK_RESULT_RETURN(commandBufferArray_.waitForAllFences(device, timeout));
            commandBufferArray_.destroy(device, commandPool);
            if (arraySize == 1)
            {
                commandBuffer_ = commandBuffer::CommandBuffer(device, commandPool, commandBufferLevel);
                type_ = Type::SINGLE;
            } else
            {
                commandBufferArray_ = commandBuffer::CommandBufferArray(device,
                                                                        commandPool,
                                                                        commandBufferLevel,
                                                                        arraySize);
            }
            break;
    }
    return VK_SUCCESS;
}

VkResult CommandBuffer::waitForAllFences(const uint64_t timeout) const
{
    switch (type_)
    {
        case Type::ARRAY:
            return commandBufferArray_.waitForAllFences(device, timeout);
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in waitForAllFences!");
    }
}

VkResult CommandBuffer::waitForFence(const uint64_t timeout) const
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.waitForFence(device, timeout);
        case Type::ARRAY:
            return commandBufferArray_.waitForFence(device, timeout);
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in waitForFence!");
    }
}

VkResult CommandBuffer::resetFence()
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.resetFence(device);
        case Type::ARRAY:
            return commandBufferArray_.resetFence(device);
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in resetFence!");
    }
}

VkResult CommandBuffer::recreateSemaphores()
{
    switch (type_)
    {
        case Type::ARRAY:
            return commandBufferArray_.recreateSemaphores();
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in recreateSemaphores!");
    }
}

VkResult CommandBuffer::ensureIsRecording(const bool shouldResetFence)
{
    if (!isRecording())
    {
        CHECK_RESULT_RETURN(waitForFence());
        if (shouldResetFence)
        {
            CHECK_RESULT_RETURN(resetFence());
        }
        CHECK_RESULT_RETURN(beginSingleUseCommandBuffer());
    }
    return VK_SUCCESS;
}
} // namespace luna

VkResult lunaAllocateCommandBuffer(const LunaCommandBufferAllocationInfo *allocationInfo,
                                   LunaCommandBuffer *commandBuffer)
{
    assert(allocationInfo);
    assert(allocationInfo->commandPool != LUNA_NULL_HANDLE);

    luna::CommandPool &commandPool = *luna::helpers::fromHandle<luna::CommandPool>(allocationInfo->commandPool);
    const uint32_t arrayCount = allocationInfo->arrayCount == 0 ? 1 : allocationInfo->arrayCount;
    CHECK_RESULT_RETURN(commandPool.allocateCommandBuffer(allocationInfo->level, arrayCount));
    *commandBuffer = luna::helpers::toHandle(commandPool.commandBuffer());
    return VK_SUCCESS;
}
