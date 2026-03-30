//
// Created by NBT22 on 6/2/25.
//

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vulkan/vulkan_core.h>
#include "commandBuffer/CommandBuffer.hpp"
#include "commandBuffer/CommandBufferArray.hpp"
#include "helpers/Handle.hpp"
#include "Semaphore.hpp"

namespace luna
{
class CommandBuffer
{
    public:
        enum class Type : uint8_t
        {
            SINGLE,
            ARRAY,
        };

        CommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel commandBufferLevel);
        CommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel commandBufferLevel, uint32_t arraySize);

        operator const VkCommandBuffer &() const;
        const VkCommandBuffer *operator&() const;

        void destroy(VkCommandPool commandPool);

        VkResult resizeArray(VkCommandPool commandPool,
                             VkCommandBufferLevel commandBufferLevel,
                             uint32_t arraySize,
                             uint64_t timeout = UINT64_MAX);
        VkResult beginSingleUseCommandBuffer();
        VkResult submitCommandBuffer(VkQueue queue, VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        VkResult submitCommandBuffer(VkQueue queue,
                                     const VkSubmitInfo &submitInfo,
                                     VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        bool getAndSetIsSignaled(bool value);
        [[nodiscard]] VkResult waitForAllFences(uint64_t timeout = UINT64_MAX) const;
        [[nodiscard]] VkResult waitForFence(uint64_t timeout = UINT64_MAX) const;
        VkResult resetFence();
        VkResult recreateSemaphores();
        VkResult ensureIsRecording(bool shouldResetFence = false);

        [[nodiscard]] bool isRecording() const;
        [[nodiscard]] const Semaphore &semaphore() const;

        [[nodiscard]] Type type() const;
        [[nodiscard]] std::string typeAsString() const;
        [[nodiscard]] const commandBuffer::CommandBuffer &commandBuffer() const;
        [[nodiscard]] const commandBuffer::CommandBufferArray &commandBufferArray() const;

    private:
        Type type_{};
        commandBuffer::CommandBuffer commandBuffer_{};
        commandBuffer::CommandBufferArray commandBufferArray_{};
};
} // namespace luna

#pragma region Implementation

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include "Luna.hpp"

namespace luna
{
inline CommandBuffer::operator const VkCommandBuffer &() const
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_;
        case Type::ARRAY:
            return commandBufferArray_;
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in operator const VkCommandBuffer &");
    }
}
inline const VkCommandBuffer *CommandBuffer::operator&() const
{
    switch (type_)
    {
        case Type::SINGLE:
            return &commandBuffer_;
        case Type::ARRAY:
            return &commandBufferArray_;
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in operator&");
    }
}

inline VkResult CommandBuffer::beginSingleUseCommandBuffer()
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.beginSingleUseCommandBuffer();
        case Type::ARRAY:
            return commandBufferArray_.beginSingleUseCommandBuffer();
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in beginSingleUseCommandBuffer!");
    }
}
inline VkResult CommandBuffer::submitCommandBuffer(const VkQueue queue, VkPipelineStageFlags stageMask)
{
    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = semaphore().isSignaled() ? 1u : 0u,
        .pWaitSemaphores = &semaphore(),
        .pWaitDstStageMask = &stageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &semaphore(),
    };
    return submitCommandBuffer(queue, submitInfo, stageMask);
}
inline VkResult CommandBuffer::submitCommandBuffer(const VkQueue queue,
                                                   const VkSubmitInfo &submitInfo,
                                                   const VkPipelineStageFlags stageMask)
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.submitCommandBuffer(queue, submitInfo, stageMask);
        case Type::ARRAY:
            return commandBufferArray_.submitCommandBuffer(queue, submitInfo, stageMask);
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in submitCommandBuffer!");
    }
}
inline bool CommandBuffer::getAndSetIsSignaled(const bool value)
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.getAndSetIsSignaled(value);
        case Type::ARRAY:
            return commandBufferArray_.getAndSetIsSignaled(value);
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in getAndSetIsSignaled!");
    }
}

inline bool CommandBuffer::isRecording() const
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.isRecording();
        case Type::ARRAY:
            return commandBufferArray_.isRecording();
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in isRecording!");
    }
}
inline const Semaphore &CommandBuffer::semaphore() const
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.semaphore();
        case Type::ARRAY:
            return commandBufferArray_.semaphore();
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in semaphore!");
    }
}
inline CommandBuffer::Type CommandBuffer::type() const
{
    return type_;
}
inline std::string CommandBuffer::typeAsString() const
{
    switch (type_)
    {
        case Type::SINGLE:
            return "Type::SINGLE";
        case Type::ARRAY:
            return "Type::ARRAY";
        default:
            return std::to_string(static_cast<std::underlying_type_t<Type>>(type_));
    }
}
inline const commandBuffer::CommandBuffer &CommandBuffer::commandBuffer() const
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_;
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in commandBuffer!");
    }
}
inline const commandBuffer::CommandBufferArray &CommandBuffer::commandBufferArray() const
{
    switch (type_)
    {
        case Type::ARRAY:
            return commandBufferArray_;
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in commandBufferArray!");
    }
}
} // namespace luna

#pragma endregion Implementation
