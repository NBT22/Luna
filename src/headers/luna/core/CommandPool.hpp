//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <luna/core/CommandBuffer.hpp>
#include <luna/lunaTypes.h>

namespace luna::core
{
class CommandPool
{
    public:
        static bool isDestroyed(const CommandPool &commandPool);

        operator const VkCommandPool &() const;

        CommandPool() = default;
        CommandPool(VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo);

        void destroy();

        VkResult allocate(VkDevice logicalDevice, const VkCommandPoolCreateInfo &poolCreateInfo);
        VkResult allocate(VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo);
        VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext);
        VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext,
                                       const VkSemaphoreCreateInfo *semaphoreCreateInfo);
        VkResult reset(VkDevice logicalDevice, VkCommandPoolResetFlagBits flags, uint64_t timeout);

        [[nodiscard]] const CommandBuffer &commandBuffer(uint32_t index = 0) const;
        [[nodiscard]] CommandBuffer &commandBuffer(uint32_t index = 0);

    private:
        bool isDestroyed_{true};
        VkCommandPool commandPool_{};
        std::vector<CommandBuffer> commandBuffers_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandPool.ipp>
