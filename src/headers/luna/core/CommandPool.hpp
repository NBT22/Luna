//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <luna/core/CommandBuffer.hpp>

namespace luna::core
{
class CommandPool
{
    public:
        CommandPool();

        void destroy(VkDevice logicalDevice);

        VkResult allocate(VkDevice logicalDevice, const VkCommandPoolCreateInfo &poolCreateInfo);
        VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext);
        VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext,
                                       const VkSemaphoreCreateInfo *semaphoreCreateInfo);

        [[nodiscard]] const CommandBuffer &commandBuffer(uint32_t index = 0) const;
        [[nodiscard]] CommandBuffer &commandBuffer(uint32_t index = 0);

    private:
        bool isDestroyed_{true};
        VkCommandPool commandPool_{};
        std::vector<CommandBuffer> commandBuffers_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandPool.ipp>
