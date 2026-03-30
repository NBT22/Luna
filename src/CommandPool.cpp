//
// Created by NBT22 on 5/19/25.
//

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaTypes.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "helpers/Handle.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna
{
CommandPool::CommandPool(const LunaCommandPoolCreationInfo &creationInfo)
{
    const VkCommandPoolCreateInfo poolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = creationInfo.flags,
        // TODO: queueFamilyIndex
    };
    CHECK_RESULT_THROW(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool_));
    isDestroyed_ = false;
}

void CommandPool::destroy()
{
    if (isDestroyed_)
    {
        return;
    }
    for (CommandBuffer &commandBuffer: commandBuffers_)
    {
        commandBuffer.destroy();
    }
    vkDestroyCommandPool(device, commandPool_, nullptr);
    isDestroyed_ = true;
}

inline VkResult CommandPool::reset(const VkCommandPoolResetFlags flags, const uint64_t timeout) const
{
    assert(!isDestroyed_);
    for (const CommandBuffer &commandBuffer: commandBuffers_)
    {
        if (commandBuffer.type() == CommandBuffer::Type::ARRAY)
        {
            CHECK_RESULT_RETURN(commandBuffer.commandBufferArray().waitForAllFences(device, timeout));
        } else
        {
            CHECK_RESULT_RETURN(commandBuffer.commandBuffer().waitForFence(device, timeout));
        }
    }
    CHECK_RESULT_RETURN(vkResetCommandPool(device, commandPool_, flags));

    return VK_SUCCESS;
}
} // namespace luna

VkResult lunaCreateCommandPool(const LunaCommandPoolCreationInfo *creationInfo, LunaCommandPool *commandPool)
{
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::device.addApplicationCommandPool(*creationInfo, commandPool));
    return VK_SUCCESS;
}

VkResult lunaResetCommandPool(const LunaCommandPool commandPool, const VkCommandPoolResetFlags flags)
{
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::CommandPool>(commandPool)->reset(flags));
    return VK_SUCCESS;
}

VkResult lunaResetCommandPoolWithTimeout(const LunaCommandPool commandPool,
                                         const VkCommandPoolResetFlags flags,
                                         const size_t timeout)
{
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::CommandPool>(commandPool)->reset(flags, timeout));
    return VK_SUCCESS;
}
