//
// Created by NBT22 on 6/2/25.
//

#pragma once

#include <algorithm>
#include <cstdint>
#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "CommandPool.hpp"
#include "Fence.hpp"
#include "helpers/Handle.hpp"
#include "Semaphore.hpp"

namespace luna
{
class CommandBuffer
{
    public:
        CommandBuffer(VkDevice device, CommandPool &commandPool, VkCommandBufferLevel commandBufferLevel);

        operator const VkCommandBuffer &() const;

        void destroy(VkDevice device);

        VkResult beginSingleUseCommandBuffer(VkDevice device);
        VkResult end();
        VkResult submit(VkDevice device, const LunaCommandBufferSubmitInfo &submitInfo);
        VkResult endAndSubmit(VkDevice device, const LunaCommandBufferSubmitInfo &submitInfo);
        VkResult waitForFence(VkDevice device, uint64_t timeout = UINT64_MAX) const;
        VkResult resetFence(VkDevice device);
        VkResult ensureIsRecording(VkDevice device);

        [[nodiscard]] bool isRecording() const;
        [[nodiscard]] CommandPool &commandPool() const;

    private:
        bool isRecording_{};
        CommandPool &commandPool_;
        VkCommandBuffer commandBuffer_{};
        Fence fence_{};
        Semaphore semaphore_{};
};
} // namespace luna

#pragma region Implementation

#include <cassert>
#include <volk.h>
#include "Luna.hpp"

namespace luna
{
inline CommandBuffer::CommandBuffer(const VkDevice device,
                                    CommandPool &commandPool,
                                    const VkCommandBufferLevel commandBufferLevel):
    commandPool_(commandPool),
    fence_(device,
           VkFenceCreateInfo{
               .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
           }),
    semaphore_(device,
               VkSemaphoreCreateInfo{
                   .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
               })
{
    const VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = commandBufferLevel,
        .commandBufferCount = 1,
    };
    CHECK_RESULT_THROW(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer_));
}

inline CommandBuffer::operator const VkCommandBuffer &() const
{
    return commandBuffer_;
}

inline void CommandBuffer::destroy(const VkDevice device)
{
    assert(!isRecording_);
    fence_.destroy(device);
    semaphore_.destroy(device);
    vkFreeCommandBuffers(device, commandPool_, 1, &commandBuffer_);
    commandBuffer_ = VK_NULL_HANDLE;
}

inline VkResult CommandBuffer::beginSingleUseCommandBuffer(const VkDevice device)
{
    assert(!isRecording_);
    CHECK_RESULT_RETURN(waitForFence(device));
    CHECK_RESULT_RETURN(resetFence(device));
    CHECK_RESULT_RETURN(vkResetCommandBuffer(commandBuffer_, 0));

    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    CHECK_RESULT_RETURN(vkBeginCommandBuffer(commandBuffer_, &commandBufferBeginInfo));
    isRecording_ = true;
    return VK_SUCCESS;
}
inline VkResult CommandBuffer::end()
{
    CHECK_RESULT_RETURN(vkEndCommandBuffer(commandBuffer_));
    isRecording_ = false;
    return VK_SUCCESS;
}
inline VkResult CommandBuffer::submit(const VkDevice device, const LunaCommandBufferSubmitInfo &submitInfo)
{
    assert(submitInfo.queue != VK_NULL_HANDLE);

    std::vector<VkSemaphore> waitSemaphores{};
    std::vector<VkPipelineStageFlags> stageMasks{};
    for (uint32_t i = 0; i < submitInfo.waitSemaphoreCount; i++)
    {
        const Semaphore *semaphore = helpers::fromHandle<Semaphore>(submitInfo.waitSemaphores[i]);
        assert(semaphore);
        waitSemaphores.emplace_back(*semaphore);
        stageMasks.emplace_back(static_cast<VkPipelineStageFlags>(submitInfo.waitDstStageMasks[i]));
    }
    if (submitInfo.waitSemaphoreCount == 0 || std::ranges::find(waitSemaphores, semaphore_) == waitSemaphores.end())
    {
        waitSemaphores.push_back(semaphore_);
        stageMasks.emplace_back(submitInfo.stageMask);
    }
    std::vector<VkSemaphore> signalSemaphores{};
    for (uint32_t i = 0; i < submitInfo.signalSemaphoreCount; i++)
    {
        const Semaphore *semaphore = helpers::fromHandle<Semaphore>(submitInfo.signalSemaphores[i]);
        assert(semaphore);
        signalSemaphores.emplace_back(*semaphore);
    }
    if (submitInfo.signalSemaphoreCount == 0 ||
        std::ranges::find(signalSemaphores, semaphore_) == signalSemaphores.end())
    {
        signalSemaphores.push_back(semaphore_);
    }

    const VkSubmitInfo vkSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = !semaphore_.isSignaled() ? 0 : static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = stageMasks.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer_,
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data(),
    };

    if (fence_.willBeSignaled())
    {
        CHECK_RESULT_RETURN(waitForFence(device));
        CHECK_RESULT_RETURN(resetFence(device));
    }
    CHECK_RESULT_RETURN(vkQueueSubmit(submitInfo.queue, 1, &vkSubmitInfo, fence_));
    fence_.setWillBeSignaled(true);
    semaphore_.setIsSignaled(true);
    semaphore_.setStageMask(submitInfo.stageMask);
    for (uint32_t i = 0; i < submitInfo.signalSemaphoreCount; i++)
    {
        assert(submitInfo.signalSemaphores[i] != LUNA_NULL_HANDLE);
        Semaphore &semaphore = *helpers::fromHandle<Semaphore>(submitInfo.signalSemaphores[i]);
        semaphore.setIsSignaled(true);
        semaphore.setStageMask(submitInfo.waitDstStageMasks[i]);
    }
    return VK_SUCCESS;
}
inline VkResult CommandBuffer::endAndSubmit(const VkDevice device, const LunaCommandBufferSubmitInfo &submitInfo)
{
    CHECK_RESULT_RETURN(end());
    CHECK_RESULT_RETURN(submit(device, submitInfo));
    return VK_SUCCESS;
}
inline VkResult CommandBuffer::waitForFence(const VkDevice device, const uint64_t timeout) const
{
    if (!fence_.willBeSignaled())
    {
        return VK_SUCCESS;
    }
    // TODO: If this fails with the default timeout it will block the the render thread for 585 years,
    //  which is unacceptable. While it is not the responsibility of this method to handle this problem,
    //  all usages of this method currently use the default timeout.
    return vkWaitForFences(device, 1, &fence_, VK_TRUE, timeout);
}
inline VkResult CommandBuffer::resetFence(const VkDevice device)
{
    fence_.setWillBeSignaled(false);
    return vkResetFences(device, 1, &fence_);
}
inline VkResult CommandBuffer::ensureIsRecording(const VkDevice device)
{
    if (!isRecording())
    {
        CHECK_RESULT_RETURN(beginSingleUseCommandBuffer(device));
    }
    return VK_SUCCESS;
}

inline bool CommandBuffer::isRecording() const
{
    return isRecording_;
}
inline CommandPool &CommandBuffer::commandPool() const
{
    return commandPool_;
}
} // namespace luna

#pragma endregion Implementation
