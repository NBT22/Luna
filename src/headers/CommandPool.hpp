//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <cstdint>
#include <list>
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

        [[nodiscard]] VkResult allocateCommandBuffer(VkCommandBufferLevel commandBufferLevel, uint32_t arraySize = 1);
        [[nodiscard]] VkResult reset(VkCommandPoolResetFlags flags, uint64_t timeout = UINT64_MAX) const;

        [[nodiscard]] size_t commandBufferCount() const;
        [[nodiscard]] const CommandBuffer &commandBuffer(uint32_t index = 0) const;
        [[nodiscard]] CommandBuffer &commandBuffer(uint32_t index = 0);

    private:
        bool isDestroyed_{true};
        VkCommandPool commandPool_{};
        std::vector<CommandBuffer *> commandBuffers_{};
        std::list<CommandBuffer> commandBufferList_{};
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

inline VkResult CommandPool::allocateCommandBuffer(VkCommandBufferLevel commandBufferLevel, const uint32_t arraySize)
{
    assert(!isDestroyed_);
    if (arraySize == 1)
    {
        TRY_CATCH_RESULT(commandBuffers_.emplace_back(&commandBufferList_.emplace_back(commandPool_,
                                                                                       commandBufferLevel)));
    } else
    {
        TRY_CATCH_RESULT(commandBuffers_.emplace_back(&commandBufferList_.emplace_back(commandPool_,
                                                                                       commandBufferLevel,
                                                                                       arraySize)));
    }
    return VK_SUCCESS;
}

inline size_t CommandPool::commandBufferCount() const
{
    return commandBuffers_.size();
}
inline const CommandBuffer &CommandPool::commandBuffer(const uint32_t index) const
{
    assert(!isDestroyed_);
    return *commandBuffers_.at(index);
}
inline CommandBuffer &CommandPool::commandBuffer(const uint32_t index)
{
    assert(!isDestroyed_);
    return *commandBuffers_.at(index);
}
} // namespace luna

#pragma endregion Implementation
