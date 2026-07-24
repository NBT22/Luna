//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace luna
{
class CommandBuffer;
class CommandPool
{
    public:
        CommandPool() = default;
        explicit CommandPool(VkDevice device, const VkCommandPoolCreateInfo *poolCreateInfo);
        explicit CommandPool(VkDevice device, const LunaCommandPoolCreationInfo &creationInfo);

        operator const VkCommandPool &() const;

        void destroy(VkDevice device);
        void destroyCommandBuffer(VkDevice device, CommandBuffer &commandBuffer);

        [[nodiscard]] VkResult allocateCommandBuffer(VkDevice device, VkCommandBufferLevel commandBufferLevel);
        [[nodiscard]] VkResult reset(VkDevice device,
                                     VkCommandPoolResetFlags flags,
                                     uint64_t timeout = UINT64_MAX) const;

        [[nodiscard]] size_t commandBufferCount() const;
        [[nodiscard]] const CommandBuffer &commandBuffer(uint32_t index = 0) const;
        [[nodiscard]] CommandBuffer &commandBuffer(uint32_t index = 0);

    private:
        bool isDestroyed_{true};
        VkCommandPool commandPool_{};
        std::vector<CommandBuffer *> commandBuffers_;
        std::list<CommandBuffer> commandBufferList_;
};
} // namespace luna

#pragma region Implementation

#include <cassert>
#include <volk.h>
#include "Luna.hpp"

namespace luna
{
inline CommandPool::CommandPool(const VkDevice device, const VkCommandPoolCreateInfo *poolCreateInfo)
{
    CHECK_RESULT_THROW(vkCreateCommandPool(device, poolCreateInfo, nullptr, &commandPool_));
    isDestroyed_ = false;
}

inline CommandPool::operator const VkCommandPool &() const
{
    assert(!isDestroyed_);
    return commandPool_;
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
