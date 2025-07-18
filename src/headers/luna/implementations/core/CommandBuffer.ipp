//
// Created by NBT22 on 7/16/25.
//

#pragma once

#include <format>
#include <stdexcept>

namespace luna::core
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
inline CommandBuffer::CommandBuffer(VkDevice logicalDevice,
                                    VkCommandPool commandPool,
                                    VkCommandBufferLevel commandBufferLevel,
                                    const void *allocateInfoPNext,
                                    uint32_t arraySize)
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

inline CommandBuffer::~CommandBuffer() {}

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
            return commandBuffer_.destroy(logicalDevice);
        case Type::ARRAY:
            return commandBufferArray_.destroy(logicalDevice);
        default:
            throw std::runtime_error("Invalid command buffer type " + typeAsString() + " when used in destroy!");
    }
}

inline void CommandBuffer::resizeArray(const VkDevice logicalDevice,
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
        return;
    }
    switch (type_)
    {
        case Type::SINGLE:
            commandBuffer_.waitForFence(logicalDevice, timeout);
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
            for (const auto isRecording: commandBufferArray_.isRecordings_)
            {
                assert(!isRecording);
            }
            commandBufferArray_.waitForAllFences(logicalDevice, timeout);
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
} // namespace luna::core
