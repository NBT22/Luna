//
// Created by NBT22 on 6/2/25.
//

#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Fence.hpp"
#include "helpers/Handle.hpp"
#include "Semaphore.hpp"

namespace luna
{
class CommandBuffer
{
    public:
        CommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel commandBufferLevel);

        operator const VkCommandBuffer &() const;

        void destroy(VkDevice device);

        VkResult beginSingleUseCommandBuffer();
        VkResult end();
        VkResult submit(VkDevice device,
                        VkQueue queue,
                        const VkSubmitInfo &submitInfo,
                        VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        VkResult endAndSubmit(VkDevice device,
                              VkQueue queue,
                              VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        VkResult endAndSubmit(VkDevice device,
                              VkQueue queue,
                              const VkSubmitInfo &submitInfo,
                              VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        bool getAndSetIsSignaled(bool value);
        VkResult waitForFence(VkDevice device, uint64_t timeout = UINT64_MAX) const;
        VkResult resetFence(VkDevice device);
        VkResult ensureIsRecording(VkDevice device, bool shouldResetFence = false);

        [[nodiscard]] bool isRecording() const;

    private:
        bool isRecording_{};
        VkCommandPool commandPool_{};
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
                                    const VkCommandPool commandPool,
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

inline VkResult CommandBuffer::beginSingleUseCommandBuffer()
{
    assert(!isRecording_);
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
inline VkResult CommandBuffer::submit(const VkDevice device,
                                      const VkQueue queue,
                                      const VkSubmitInfo &submitInfo,
                                      const VkPipelineStageFlags stageMask)
{
    if (fence_.willBeSignaled())
    {
        CHECK_RESULT_RETURN(waitForFence(device));
        CHECK_RESULT_RETURN(resetFence(device));
    }
    CHECK_RESULT_RETURN(vkQueueSubmit(queue, 1, &submitInfo, fence_));
    fence_.setWillBeSignaled(true);
    if (submitInfo.signalSemaphoreCount > 0)
    {
        for (uint32_t i = 0; i < submitInfo.signalSemaphoreCount; i++)
        {
            if (submitInfo.pSignalSemaphores[i] == semaphore_)
            {
                semaphore_.setIsSignaled(true);
                semaphore_.setStageMask(stageMask);
                break;
            }
        }
    }
    return VK_SUCCESS;
}
inline VkResult CommandBuffer::endAndSubmit(const VkDevice device, const VkQueue queue, VkPipelineStageFlags stageMask)
{
    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = semaphore_.isSignaled() ? 1u : 0u,
        .pWaitSemaphores = &semaphore_,
        .pWaitDstStageMask = &stageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &static_cast<const VkCommandBuffer &>(*this),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &semaphore_,
    };
    return endAndSubmit(device, queue, submitInfo, stageMask);
}
inline VkResult CommandBuffer::endAndSubmit(const VkDevice device,
                                            const VkQueue queue,
                                            const VkSubmitInfo &submitInfo,
                                            const VkPipelineStageFlags stageMask)
{
    CHECK_RESULT_RETURN(end());
    CHECK_RESULT_RETURN(submit(device, queue, submitInfo, stageMask));
    return VK_SUCCESS;
}
inline bool CommandBuffer::getAndSetIsSignaled(const bool value)
{
    const bool oldValue = semaphore_.isSignaled();
    semaphore_.setIsSignaled(value);
    return oldValue;
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
inline VkResult CommandBuffer::ensureIsRecording(const VkDevice device, const bool shouldResetFence)
{
    if (!isRecording())
    {
        CHECK_RESULT_RETURN(waitForFence(device));
        if (shouldResetFence)
        {
            CHECK_RESULT_RETURN(resetFence(device));
        }
        CHECK_RESULT_RETURN(beginSingleUseCommandBuffer());
    }
    return VK_SUCCESS;
}

inline bool CommandBuffer::isRecording() const
{
    return isRecording_;
}
} // namespace luna

#pragma endregion Implementation
