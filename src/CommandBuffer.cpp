//
// Created by NBT22 on 3/28/26.
//

#include <cassert>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaTypes.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "CommandPool.hpp"
#include "helpers/Handle.hpp"
#include "Luna.hpp"

VkResult lunaAllocateCommandBuffer(const LunaDevice device,
                                   const LunaCommandPool commandPool,
                                   const VkCommandBufferLevel level,
                                   LunaCommandBuffer *commandBuffer)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandPool != LUNA_NULL_HANDLE);

    luna::CommandPool &commandPoolObject = *luna::helpers::fromHandle<luna::CommandPool>(commandPool);
    CHECK_RESULT_RETURN(commandPoolObject.allocateCommandBuffer(lunaGetVkDevice(device), level));
    if (commandBuffer != nullptr)
    {
        *commandBuffer =
                luna::helpers::toHandle(&commandPoolObject.commandBuffer(commandPoolObject.commandBufferCount() - 1));
    }
    return VK_SUCCESS;
}

VkResult lunaBeginSingleUseCommandBuffer(const LunaDevice device, const LunaCommandBuffer commandBuffer)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);

    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    if (!commandBufferObject.isRecording())
    {
        CHECK_RESULT_RETURN(commandBufferObject.beginSingleUseCommandBuffer(lunaGetVkDevice(device)));
    }
    return VK_SUCCESS;
}

VkResult lunaEndCommandBuffer(const LunaCommandBuffer commandBuffer)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.end());
    return VK_SUCCESS;
}

VkResult lunaSubmitCommandBuffer(const LunaDevice device,
                                 const LunaCommandBuffer commandBuffer,
                                 const LunaCommandBufferSubmitInfo *submitInfo)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(submitInfo);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.submit(static_cast<
                                                           VkDevice>(*luna::helpers::fromHandle<luna::Device>(device)),
                                                   *submitInfo));
    return VK_SUCCESS;
}

VkResult lunaEndAndSubmitCommandBuffer(const LunaDevice device,
                                       const LunaCommandBuffer commandBuffer,
                                       const LunaCommandBufferSubmitInfo *submitInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(submitInfo);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.endAndSubmit(static_cast<VkDevice>(*luna::helpers::fromHandle<
                                                                               luna::Device>(device)),
                                                         *submitInfo));
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

void lunaDestroyCommandBuffer(const LunaDevice device, const LunaCommandBuffer commandBuffer)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);

    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    luna::CommandPool &commandPool = commandBufferObject.commandPool();
    commandPool.destroyCommandBuffer(lunaGetVkDevice(device), commandBufferObject);
}
