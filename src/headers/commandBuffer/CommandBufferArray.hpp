//
// Created by NBT22 on 5/29/25.
//

#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Fence.hpp"
#include "Semaphore.hpp"

namespace luna
{
class CommandBuffer;
} // namespace luna

namespace luna::commandBuffer
{
class CommandBufferArray
{
    public:
        CommandBufferArray() = default;
        CommandBufferArray(VkDevice device,
                           VkCommandPool commandPool,
                           VkCommandBufferLevel commandBufferLevel,
                           uint32_t count);

        operator const VkCommandBuffer &() const;
        const VkCommandBuffer *operator&() const;

        void destroy(VkDevice device, VkCommandPool commandPool);

        VkResult beginSingleUseCommandBuffer();
        VkResult end();
        VkResult submit(VkQueue queue,
                        const VkSubmitInfo &submitInfo,
                        VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        VkResult endAndSubmit(VkQueue queue,
                              const VkSubmitInfo &submitInfo,
                              VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        bool getAndSetIsSignaled(bool value);
        VkResult waitForAllFences(VkDevice device, uint64_t timeout = UINT64_MAX) const;
        VkResult waitForFence(VkDevice device, uint64_t timeout = UINT64_MAX) const;
        VkResult resetFence(VkDevice device);
        VkResult recreateSemaphores(VkDevice device);

        [[nodiscard]] size_t size() const;
        [[nodiscard]] bool isRecording() const;
        [[nodiscard]] bool anyRecording() const;
        [[nodiscard]] const Semaphore &semaphore() const;

    private:
        uint32_t index_{};
        std::vector<uint8_t> isRecordings_{};
        std::vector<VkCommandBuffer> commandBuffers_{};
        std::vector<Fence> fences_{};
        std::vector<Semaphore> semaphores_{};
};
} // namespace luna::commandBuffer

#pragma region Implementation

#include <algorithm>
#include <cassert>
#include <cstddef>
#include "Luna.hpp"

namespace luna::commandBuffer
{
inline CommandBufferArray::CommandBufferArray(const VkDevice device,
                                              const VkCommandPool commandPool,
                                              const VkCommandBufferLevel commandBufferLevel,
                                              const uint32_t count)
{
    commandBuffers_.resize(count);
    isRecordings_.resize(count);
    const VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = commandBufferLevel,
        .commandBufferCount = count,
    };
    CHECK_RESULT_THROW(vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers_.data()));

    fences_.reserve(count);
    semaphores_.reserve(count);
    constexpr VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    for (uint32_t i = 0; i < count; i++)
    {
        fences_.emplace_back(device, fenceCreateInfo);
        semaphores_.emplace_back(device, semaphoreCreateInfo);
    }
}

inline CommandBufferArray::operator const VkCommandBuffer &() const
{
    return commandBuffers_.at(index_);
}
inline const VkCommandBuffer *CommandBufferArray::operator&() const
{
    return &commandBuffers_.at(index_);
}

inline void CommandBufferArray::destroy(const VkDevice device, const VkCommandPool commandPool)
{
    assert(std::ranges::all_of(isRecordings_, [](const uint8_t val) -> bool { return val == 0; }));
    for (Fence &fence: fences_)
    {
        fence.destroy(device);
    }
    fences_.clear();
    for (Semaphore &semaphore: semaphores_)
    {
        semaphore.destroy(device);
    }
    semaphores_.clear();
    vkFreeCommandBuffers(device, commandPool, commandBuffers_.size(), commandBuffers_.data());
}

inline VkResult CommandBufferArray::beginSingleUseCommandBuffer()
{
    assert(!isRecordings_.at(index_));
    CHECK_RESULT_RETURN(vkResetCommandBuffer(commandBuffers_.at(index_), 0));

    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    CHECK_RESULT_RETURN(vkBeginCommandBuffer(commandBuffers_.at(index_), &commandBufferBeginInfo));
    isRecordings_.at(index_) = 1u;
    return VK_SUCCESS;
}
inline VkResult CommandBufferArray::end()
{
    CHECK_RESULT_RETURN(vkEndCommandBuffer(commandBuffers_.at(index_)));
    isRecordings_.at(index_) = static_cast<uint8_t>(0);
    return VK_SUCCESS;
}
inline VkResult CommandBufferArray::submit(const VkQueue queue,
                                           const VkSubmitInfo &submitInfo,
                                           const VkPipelineStageFlags stageMask)
{
    CHECK_RESULT_RETURN(vkQueueSubmit(queue, 1, &submitInfo, fences_.at(index_)));
    fences_.at(index_).setWillBeSignaled(true);
    if (submitInfo.signalSemaphoreCount > 0)
    {
        for (uint32_t i = 0; i < submitInfo.signalSemaphoreCount; i++)
        {
            if (submitInfo.pSignalSemaphores[i] == semaphores_.at(index_))
            {
                semaphores_.at(index_).setIsSignaled(true);
                semaphores_.at(index_).setStageMask(stageMask);
                break;
            }
        }
    }
    index_ = (index_ + 1) % commandBuffers_.size();
    return VK_SUCCESS;
}
inline VkResult CommandBufferArray::endAndSubmit(const VkQueue queue,
                                                 const VkSubmitInfo &submitInfo,
                                                 const VkPipelineStageFlags stageMask)
{
    CHECK_RESULT_RETURN(end());
    CHECK_RESULT_RETURN(submit(queue, submitInfo, stageMask));
    return VK_SUCCESS;
}
inline bool CommandBufferArray::getAndSetIsSignaled(const bool value)
{
    const bool oldValue = semaphores_.at(index_).isSignaled();
    semaphores_.at(index_).setIsSignaled(value);
    return oldValue;
}
inline VkResult CommandBufferArray::waitForAllFences(const VkDevice device, const uint64_t timeout) const
{
    std::vector<VkFence> fences;
    fences.reserve(fences_.size());
    uint32_t waitCount = 0;
    for (const Fence &fence: fences_)
    {
        if (fence.willBeSignaled())
        {
            fences.emplace_back(fence);
            waitCount++;
        }
    }
    if (waitCount == 0)
    {
        return VK_SUCCESS;
    }
    return vkWaitForFences(device, waitCount, fences.data(), VK_TRUE, timeout);
}
inline VkResult CommandBufferArray::waitForFence(const VkDevice device, const uint64_t timeout) const
{
    if (!fences_.at(index_).willBeSignaled())
    {
        return VK_SUCCESS;
    }
    // TODO: If this fails with the default timeout it will block the the render thread for 585 years,
    //  which is unacceptable. While it is not the responsibility of this method to handle this problem,
    //  all usages of this method currently use the default timeout.
    return vkWaitForFences(device, 1, &fences_.at(index_), VK_TRUE, timeout);
}
inline VkResult CommandBufferArray::resetFence(const VkDevice device)
{
    fences_.at(index_).setWillBeSignaled(false);
    return vkResetFences(device, 1, &fences_.at(index_));
}
inline VkResult CommandBufferArray::recreateSemaphores(const VkDevice device)
{
    const size_t count = semaphores_.size();
    semaphores_.clear();
    for (size_t i = 0; i < count; i++)
    {
        semaphores_.emplace_back();
        CHECK_RESULT_RETURN(semaphores_.back().create(device));
    }
    return VK_SUCCESS;
}

inline size_t CommandBufferArray::size() const
{
    const size_t size = commandBuffers_.size();
    assert(isRecordings_.size() == size && fences_.size() == size && semaphores_.size() == size);
    return size;
}
inline bool CommandBufferArray::isRecording() const
{
    return isRecordings_.at(index_) != 0u;
}
inline bool CommandBufferArray::anyRecording() const
{
    return std::ranges::any_of(isRecordings_, [](const uint8_t val) -> bool { return val != 0u; });
}
inline const Semaphore &CommandBufferArray::semaphore() const
{
    return semaphores_.at(index_);
}
} // namespace luna::commandBuffer

#pragma endregion Implementation
