//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <luna/core/CommandBuffer/CommandBuffer.hpp>
#include <luna/core/CommandBuffer/CommandBufferArray.hpp>
#include <luna/lunaTypes.h>
#include <memory>

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
        template<uint32_t arraySize = 1> VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext);
        template<uint32_t arraySize = 1> VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                       VkCommandBufferLevel commandBufferLevel,
                                       const void *allocateInfoPNext,
                                       const VkSemaphoreCreateInfo *semaphoreCreateInfo);
        VkResult reset(VkDevice logicalDevice, VkCommandPoolResetFlagBits flags, uint64_t timeout = UINT64_MAX);

        [[nodiscard]] const CommandBuffer &commandBuffer(uint32_t index = 0) const;
        [[nodiscard]] CommandBuffer &commandBuffer(uint32_t index = 0);

    private:
        bool isDestroyed_{true};
        VkCommandPool commandPool_{};
        std::vector<std::unique_ptr<CommandBuffer>> commandBuffers_{};
};
} // namespace luna::core

#include <luna/implementations/core/CommandPool.ipp>
