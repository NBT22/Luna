//
// Created by NBT22 on 6/2/25.
//

#pragma once

#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>
#include "commandBuffer/CommandBuffer.hpp"
#include "commandBuffer/CommandBufferArray.hpp"
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

        CommandBuffer(const CommandBuffer &other);
        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext,
                      uint32_t arraySize);
        CommandBuffer(VkDevice logicalDevice,
                      VkCommandPool commandPool,
                      VkCommandBufferLevel commandBufferLevel,
                      const void *allocateInfoPNext,
                      const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                      uint32_t arraySize);

        operator const VkCommandBuffer &() const;
        const VkCommandBuffer *operator&() const;

        void destroy(VkDevice logicalDevice) const;

        VkResult resizeArray(VkDevice logicalDevice,
                             VkCommandPool commandPool,
                             VkCommandBufferLevel commandBufferLevel,
                             const void *allocateInfoPNext,
                             const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                             uint32_t arraySize,
                             uint64_t timeout = UINT64_MAX);
        VkResult beginSingleUseCommandBuffer();
        VkResult submitCommandBuffer(VkQueue queue,
                                     const VkSubmitInfo &submitInfo,
                                     VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        bool getAndSetIsSignaled(bool value);
        VkResult waitForAllFences(VkDevice logicalDevice, uint64_t timeout = UINT64_MAX) const;
        VkResult waitForFence(VkDevice logicalDevice, uint64_t timeout = UINT64_MAX) const;
        VkResult resetFence(VkDevice logicalDevice);
        VkResult recreateSemaphores(VkDevice logicalDevice);
        VkResult ensureIsRecording(VkDevice logicalDevice, bool shouldResetFence = false);

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

#pragma region "Implmentation"

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include "Luna.hpp"

namespace luna
{
inline CommandBuffer::CommandBuffer(const CommandBuffer &other)
{
    type_ = other.type_;
    switch (type_)
    {
        case Type::SINGLE:
            commandBuffer_ = other.commandBuffer_;
            break;
        case Type::ARRAY:
            commandBufferArray_ = other.commandBufferArray_;
            break;
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in copy constructor!");
    }
}
inline CommandBuffer::CommandBuffer(const VkDevice logicalDevice,
                                    const VkCommandPool commandPool,
                                    const VkCommandBufferLevel commandBufferLevel,
                                    const void *allocateInfoPNext,
                                    const uint32_t arraySize)
{
    assert(arraySize > 0);
    if (arraySize == 1)
    {
        type_ = Type::SINGLE;
        commandBuffer_ = commandBuffer::CommandBuffer(logicalDevice,
                                                      commandPool,
                                                      commandBufferLevel,
                                                      allocateInfoPNext);
    } else
    {
        type_ = Type::ARRAY;
        commandBufferArray_ = commandBuffer::CommandBufferArray(logicalDevice,
                                                                commandPool,
                                                                commandBufferLevel,
                                                                allocateInfoPNext,
                                                                arraySize);
    }
}
inline CommandBuffer::CommandBuffer(const VkDevice logicalDevice,
                                    const VkCommandPool commandPool,
                                    const VkCommandBufferLevel commandBufferLevel,
                                    const void *allocateInfoPNext,
                                    const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                                    const uint32_t arraySize)
{
    assert(arraySize > 0);
    if (arraySize == 1)
    {
        type_ = Type::SINGLE;
        commandBuffer_ = commandBuffer::CommandBuffer(logicalDevice,
                                                      commandPool,
                                                      commandBufferLevel,
                                                      allocateInfoPNext,
                                                      semaphoreCreateInfo);
    } else
    {
        type_ = Type::ARRAY;
        commandBufferArray_ = commandBuffer::CommandBufferArray(logicalDevice,
                                                                commandPool,
                                                                commandBufferLevel,
                                                                allocateInfoPNext,
                                                                semaphoreCreateInfo,
                                                                arraySize);
    }
}

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

inline void CommandBuffer::destroy(const VkDevice logicalDevice) const
{
    switch (type_)
    {
        case Type::SINGLE:
            commandBuffer_.destroy(logicalDevice);
            break;
        case Type::ARRAY:
            commandBufferArray_.destroy(logicalDevice);
            break;
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in destroy!");
    }
}

inline VkResult CommandBuffer::resizeArray(const VkDevice logicalDevice,
                                           const VkCommandPool commandPool,
                                           const VkCommandBufferLevel commandBufferLevel,
                                           const void *allocateInfoPNext,
                                           const VkSemaphoreCreateInfo *semaphoreCreateInfo,
                                           const uint32_t arraySize,
                                           const uint64_t timeout)
{
    if ((type_ == Type::SINGLE && arraySize == 1) ||
        (type_ == Type::ARRAY && arraySize == commandBufferArray_.commandBuffers_.size()))
    {
        return VK_SUCCESS;
    }
    switch (type_)
    {
        case Type::SINGLE:
            CHECK_RESULT_RETURN(commandBuffer_.waitForFence(logicalDevice, timeout));
            commandBuffer_.destroy(logicalDevice, commandPool);
            commandBufferArray_ = commandBuffer::CommandBufferArray(logicalDevice,
                                                                    commandPool,
                                                                    commandBufferLevel,
                                                                    allocateInfoPNext,
                                                                    semaphoreCreateInfo,
                                                                    arraySize);
            type_ = Type::ARRAY;
            break;
        case Type::ARRAY:
            for ([[maybe_unused]] const auto isRecording: commandBufferArray_.isRecordings_)
            {
                assert(!isRecording);
            }
            CHECK_RESULT_RETURN(commandBufferArray_.waitForAllFences(logicalDevice, timeout));
            commandBufferArray_.destroy(logicalDevice, commandPool);
            if (arraySize == 1)
            {
                commandBuffer_ = commandBuffer::CommandBuffer(logicalDevice,
                                                              commandPool,
                                                              commandBufferLevel,
                                                              allocateInfoPNext,
                                                              semaphoreCreateInfo);
                type_ = Type::SINGLE;
            } else
            {
                commandBufferArray_ = commandBuffer::CommandBufferArray(logicalDevice,
                                                                        commandPool,
                                                                        commandBufferLevel,
                                                                        allocateInfoPNext,
                                                                        semaphoreCreateInfo,
                                                                        arraySize);
            }
            break;
    }
    return VK_SUCCESS;
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
inline VkResult CommandBuffer::waitForAllFences(const VkDevice logicalDevice, const uint64_t timeout) const
{
    switch (type_)
    {
        case Type::ARRAY:
            return commandBufferArray_.waitForAllFences(logicalDevice, timeout);
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in waitForAllFences!");
    }
}
inline VkResult CommandBuffer::waitForFence(const VkDevice logicalDevice, const uint64_t timeout) const
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.waitForFence(logicalDevice, timeout);
        case Type::ARRAY:
            return commandBufferArray_.waitForFence(logicalDevice, timeout);
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in waitForFence!");
    }
}
inline VkResult CommandBuffer::resetFence(const VkDevice logicalDevice)
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_.resetFence(logicalDevice);
        case Type::ARRAY:
            return commandBufferArray_.resetFence(logicalDevice);
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in resetFence!");
    }
}
inline VkResult CommandBuffer::recreateSemaphores(const VkDevice logicalDevice)
{
    switch (type_)
    {
        case Type::ARRAY:
            return commandBufferArray_.recreateSemaphores(logicalDevice);
        default:
            throw std::runtime_error("Invalid command buffer type " +
                                     typeAsString() +
                                     " when used in recreateSemaphores!");
    }
}
inline VkResult CommandBuffer::ensureIsRecording(const VkDevice logicalDevice, const bool shouldResetFence)
{
    if (!isRecording())
    {
        CHECK_RESULT_RETURN(waitForFence(logicalDevice));
        if (shouldResetFence)
        {
            CHECK_RESULT_RETURN(resetFence(logicalDevice));
        }
        CHECK_RESULT_RETURN(beginSingleUseCommandBuffer());
    }
    return VK_SUCCESS;
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

#pragma endregion "Implmentation"
