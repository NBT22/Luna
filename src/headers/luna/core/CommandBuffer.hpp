//
// Created by NBT22 on 3/20/25.
//

#pragma once

#include <vulkan/vulkan_core.h>

namespace luna::core
{
class CommandBuffer
{
    public:
        friend class CommandPool;

        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext);

        operator const VkCommandBuffer&() const;
        const VkCommandBuffer *operator &() const;

        VkResult beginSingleUseCommandBuffer();
        VkResult submitCommandBuffer(VkQueue queue, const VkSubmitInfo &submitInfo);
        void setRecording(bool value);
        VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout) const;
        VkResult resetFence(VkDevice logicalDevice) const;

        [[nodiscard]] bool isRecording() const;

    private:
        bool isRecording_{};
        VkCommandBuffer commandBuffer_{};
        VkFence fence_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandBuffer.ipp>
