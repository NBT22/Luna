//
// Created by NBT22 on 3/28/26.
//

#include <cassert>
#include <cstdint>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaTypes.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Fence.hpp"
#include "helpers/Handle.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna
{
CommandBuffer::CommandBuffer(const VkCommandPool commandPool, const VkCommandBufferLevel commandBufferLevel):
    type_(Type::SINGLE),
    commandPool_(commandPool),
    commandBuffer_(device, commandPool, commandBufferLevel)
{}
CommandBuffer::CommandBuffer(const VkCommandPool commandPool,
                             const VkCommandBufferLevel commandBufferLevel,
                             const uint32_t arraySize):
    type_(Type::ARRAY),
    commandPool_(commandPool),
    commandBufferArray_(device, commandPool, commandBufferLevel, arraySize)
{
    assert(arraySize > 1);
}

void CommandBuffer::destroy()
{
    switch (type_)
    {
        case Type::SINGLE:
            commandBuffer_.destroy(device, commandPool_);
            break;
        case Type::ARRAY:
            commandBufferArray_.destroy(device, commandPool_);
            break;
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in destroy");
    }
}


VkResult CommandBuffer::resizeArray(const VkCommandBufferLevel commandBufferLevel,
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
            commandBuffer_.destroy(device, commandPool_);
            commandBufferArray_ = commandBuffer::CommandBufferArray(device,
                                                                    commandPool_,
                                                                    commandBufferLevel,
                                                                    arraySize);
            type_ = Type::ARRAY;
            break;
        case Type::ARRAY:
            assert(!commandBufferArray_.anyRecording());
            CHECK_RESULT_RETURN(commandBufferArray_.waitForAllFences(device, timeout));
            commandBufferArray_.destroy(device, commandPool_);
            if (arraySize == 1)
            {
                commandBuffer_ = commandBuffer::CommandBuffer(device, commandPool_, commandBufferLevel);
                type_ = Type::SINGLE;
            } else
            {
                commandBufferArray_ = commandBuffer::CommandBufferArray(device,
                                                                        commandPool_,
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

    luna::CommandPool &commandPool = *luna::helpers::fromHandle<luna::CommandPool>(*allocationInfo->commandPool);
    const uint32_t arrayCount = allocationInfo->arrayCount == 0 ? 1 : allocationInfo->arrayCount;
    CHECK_RESULT_RETURN(commandPool.allocateCommandBuffer(allocationInfo->level, arrayCount));
    *commandBuffer = luna::helpers::toHandle(&commandPool.commandBuffer(commandPool.commandBufferCount() - 1));
    return VK_SUCCESS;
}

VkResult lunaBeginSingleUseCommandBuffer(const LunaCommandBuffer commandBuffer)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);

    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.beginSingleUseCommandBuffer());
    return VK_SUCCESS;
}

VkResult lunaEndCommandBuffer(const LunaCommandBuffer commandBuffer)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);

    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.end());
    return VK_SUCCESS;
}

VkResult lunaResetCommandBuffer(const LunaCommandBuffer commandBuffer, const VkCommandBufferResetFlags flags)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);

    const luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(vkResetCommandBuffer(commandBufferObject, flags));
    return VK_SUCCESS;
}

VkCommandBuffer lunaGetVkCommandBuffer(const LunaCommandBuffer commandBuffer)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);
    return *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
}

void lunaDestroyCommandBuffer(const LunaCommandBuffer commandBuffer)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);

    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    commandBufferObject.destroy();
    // TODO (0.3.0): Wherever the handle is a pointer to should get freed
}
