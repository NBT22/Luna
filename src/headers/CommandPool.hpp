//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <cstdint>
#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"

namespace luna
{
class CommandPool
{
    public:
        CommandPool() = default;
        explicit CommandPool(VkDevice logicalDevice, const VkCommandPoolCreateInfo *poolCreateInfo);
        explicit CommandPool(const LunaCommandPoolCreationInfo &creationInfo);

        operator const VkCommandPool &() const;

        void destroy();

        [[nodiscard]] VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                                     VkCommandBufferLevel commandBufferLevel,
                                                     const void *allocateInfoPNext,
                                                     uint32_t arraySize = 1);
        [[nodiscard]] VkResult allocateCommandBuffer(VkDevice logicalDevice,
                                                     VkCommandBufferLevel commandBufferLevel,
                                                     const void *allocateInfoPNext,
                                                     const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                                                     uint32_t arraySize = 1);
        [[nodiscard]] VkResult reset(VkCommandPoolResetFlags flags, uint64_t timeout = UINT64_MAX) const;

        [[nodiscard]] const CommandBuffer &commandBuffer(uint32_t index = 0) const;
        [[nodiscard]] CommandBuffer &commandBuffer(uint32_t index = 0);

    private:
        bool isDestroyed_{true};
        VkCommandPool commandPool_{};
        std::vector<CommandBuffer> commandBuffers_{};
};
} // namespace luna

#pragma region Implementation

#include <cassert>
#include <volk.h>
#include "Luna.hpp"

namespace luna
{
inline CommandPool::CommandPool(const VkDevice logicalDevice, const VkCommandPoolCreateInfo *poolCreateInfo)
{
    CHECK_RESULT_THROW(vkCreateCommandPool(logicalDevice, poolCreateInfo, nullptr, &commandPool_));
    isDestroyed_ = false;
}

inline CommandPool::operator const VkCommandPool &() const
{
    assert(!isDestroyed_);
    return commandPool_;
}

inline VkResult CommandPool::allocateCommandBuffer(const VkDevice logicalDevice,
                                                   VkCommandBufferLevel commandBufferLevel,
                                                   const void *allocateInfoPNext,
                                                   const uint32_t arraySize)
{
    assert(!isDestroyed_);
    TRY_CATCH_RESULT(commandBuffers_.emplace_back(logicalDevice,
                                                  commandPool_,
                                                  commandBufferLevel,
                                                  allocateInfoPNext,
                                                  arraySize));
    return VK_SUCCESS;
}
inline VkResult CommandPool::allocateCommandBuffer(const VkDevice logicalDevice,
                                                   VkCommandBufferLevel commandBufferLevel,
                                                   const void *allocateInfoPNext,
                                                   const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                                                   const uint32_t arraySize)
{
    assert(!isDestroyed_);
    TRY_CATCH_RESULT(commandBuffers_.emplace_back(logicalDevice,
                                                  commandPool_,
                                                  commandBufferLevel,
                                                  allocateInfoPNext,
                                                  semaphoreCreateInfo,
                                                  arraySize));
    return VK_SUCCESS;
}

inline const CommandBuffer &CommandPool::commandBuffer(const uint32_t index) const
{
    assert(!isDestroyed_);
    return commandBuffers_.at(index);
}
inline CommandBuffer &CommandPool::commandBuffer(const uint32_t index)
{
    assert(!isDestroyed_);
    return commandBuffers_.at(index);
}
} // namespace luna

#pragma endregion Implementation
