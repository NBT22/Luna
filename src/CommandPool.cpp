//
// Created by NBT22 on 5/19/25.
//

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaTypes.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "helpers/Handle.hpp"
#include "Luna.hpp"

namespace luna
{
CommandPool::CommandPool(const VkDevice device, const LunaCommandPoolCreationInfo &creationInfo)
{
    const VkCommandPoolCreateInfo poolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = creationInfo.flags | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = creationInfo.queueFamilyIndex,
    };
    CHECK_RESULT_THROW(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool_));
    isDestroyed_ = false;
}

void CommandPool::destroy(const VkDevice device)
{
    if (isDestroyed_)
    {
        return;
    }
    commandBuffers_.clear();
    for (CommandBuffer &commandBuffer: commandBufferList_)
    {
        commandBuffer.destroy(device);
    }
    vkDestroyCommandPool(device, commandPool_, nullptr);
    isDestroyed_ = true;
}

inline VkResult CommandPool::reset(const VkDevice device,
                                   const VkCommandPoolResetFlags flags,
                                   const uint64_t timeout) const
{
    assert(!isDestroyed_);
    for (const CommandBuffer &commandBuffer: commandBufferList_)
    {
        CHECK_RESULT_RETURN(commandBuffer.waitForFence(device, timeout));
    }
    CHECK_RESULT_RETURN(vkResetCommandPool(device, commandPool_, flags));

    return VK_SUCCESS;
}
} // namespace luna

VkResult lunaCreateCommandPool(const LunaDevice device,
                               const LunaCommandPoolCreationInfo *creationInfo,
                               LunaCommandPool *commandPool)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createCommandPool(*creationInfo, commandPool));
    return VK_SUCCESS;
}

VkResult lunaResetCommandPool(const LunaDevice device,
                              const LunaCommandPool commandPool,
                              const VkCommandPoolResetFlags flags)
{
    CHECK_RESULT_RETURN(
            luna::helpers::fromHandle<luna::CommandPool>(commandPool)->reset(lunaGetVkDevice(device), flags));
    return VK_SUCCESS;
}

VkResult lunaResetCommandPoolWithTimeout(const LunaDevice device,
                                         const LunaCommandPool commandPool,
                                         const VkCommandPoolResetFlags flags,
                                         const size_t timeout)
{
    CHECK_RESULT_RETURN(
            luna::helpers::fromHandle<luna::CommandPool>(commandPool)->reset(lunaGetVkDevice(device), flags, timeout));
    return VK_SUCCESS;
}
