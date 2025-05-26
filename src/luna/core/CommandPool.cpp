//
// Created by NBT22 on 5/19/25.
//

#include <luna/core/CommandPool.hpp>
#include <luna/core/Instance.hpp>

namespace luna::core
{
void CommandPool::destroy()
{
    // It's literally just wrong.
    // ReSharper disable CppDFAConstantConditions CppDFAUnreachableCode
    if (isDestroyed_)
    {
        return;
    }
    for (const CommandBuffer &commandBuffer: commandBuffers_)
    {
        assert(!commandBuffer.isRecording_);
        vkDestroyFence(device, commandBuffer.fence_, nullptr);
        commandBuffer.semaphore_.destroy(device);
    }
    vkDestroyCommandPool(device, commandPool_, nullptr);
    isDestroyed_ = true;
    // ReSharper restore CppDFAConstantConditions CppDFAUnreachableCode
}
} // namespace luna::core

VkResult lunaCreateCommandPool(const LunaCommandPoolCreationInfo *creationInfo, LunaCommandPool *commandPool)
{
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::core::device.addApplicationCommandPool(*creationInfo, commandPool));
    return VK_SUCCESS;
}

VkResult lunaResetCommandPool(const LunaCommandPool commandPool, const VkCommandPoolResetFlagBits flags)
{
    switch (reinterpret_cast<uintptr_t>(commandPool))
    {
        case LUNA_INTERNAL_GRAPHICS_COMMAND_POOL:
            CHECK_RESULT_RETURN(luna::core::device.commandPools().graphics.reset(luna::core::device, flags));
            return VK_SUCCESS;
        default:
            return VK_ERROR_UNKNOWN;
    }
}
