//
// Created by NBT22 on 6/2/25.
//

#pragma once

#include <luna/core/Semaphore.hpp>

namespace luna::core
{
class CommandBuffer
{
    public:
        virtual ~CommandBuffer() = default;

        virtual operator const VkCommandBuffer &() const = 0;
        virtual const VkCommandBuffer *operator&() const = 0;

        virtual void destroy(VkDevice logicalDevice) const = 0;

        virtual VkResult beginSingleUseCommandBuffer() = 0;
        virtual VkResult submitCommandBuffer(VkQueue queue,
                                             const VkSubmitInfo &submitInfo,
                                             VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) = 0;
        virtual bool getAndSetIsSignaled(bool value) = 0;
        virtual VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout = UINT64_MAX) const = 0;
        virtual VkResult resetFence(VkDevice logicalDevice) = 0;

        [[nodiscard]] virtual bool isRecording() const = 0;
        [[nodiscard]] virtual const Semaphore &semaphore() const = 0;
};
} // namespace luna::core
