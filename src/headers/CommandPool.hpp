//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include "CommandBuffer.hpp"

namespace luna::core
{
class CommandPool
{
    public:
        static bool isDestroyed(const CommandPool &commandPool);

        operator const VkCommandPool &() const;

        CommandPool() = default;
        CommandPool(VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo);

        void destroy(VkDevice logicalDevice);

        VkResult allocate(VkDevice logicalDevice, const VkCommandPoolCreateInfo &poolCreateInfo);
        VkResult allocate(VkDevice logicalDevice, const LunaCommandPoolCreationInfo &creationInfo);
        VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext,
                                       uint32_t arraySize = 1);
        VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext,
                                       const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                                       uint32_t arraySize = 1);
        VkResult reset(VkDevice logicalDevice, VkCommandPoolResetFlagBits flags, uint64_t timeout = UINT64_MAX);

        [[nodiscard]] const CommandBuffer &commandBuffer(uint32_t index = 0) const;
        [[nodiscard]] CommandBuffer &commandBuffer(uint32_t index = 0);

    private:
        bool isDestroyed_{true};
        VkCommandPool commandPool_{};
        std::vector<CommandBuffer> commandBuffers_{};
};
} // namespace luna::core

#include "implementations/CommandPool.ipp"
