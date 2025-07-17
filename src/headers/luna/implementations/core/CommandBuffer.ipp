//
// Created by NBT22 on 7/16/25.
//

#pragma once

#include <stdexcept>

namespace luna::core
{
inline CommandBuffer::CommandBuffer(const CommandBuffer &commandBuffer)
{
    type_ = commandBuffer.type_;
    switch (type_)
    {
        case Type::SINGLE:
            commandBuffer_ = commandBuffer.commandBuffer_;
            break;
        case Type::ARRAY:
            commandBufferArray_ = commandBuffer.commandBufferArray_;
            break;
        default:
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
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

inline CommandBuffer::~CommandBuffer()
{
    // throw std::runtime_error("implement me please :(");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
    }
}
inline VkResult CommandBuffer::waitForAllFences(const VkDevice logicalDevice, const uint64_t timeout) const
{
    switch (type_)
    {
        case Type::ARRAY:
            return commandBufferArray_.waitForAllFences(logicalDevice, timeout);
        default:
            throw std::runtime_error("type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
    }
}
inline VkResult CommandBuffer::recreateSemaphores(const VkDevice logicalDevice)
{
    switch (type_)
    {
        case Type::ARRAY:
            return commandBufferArray_.recreateSemaphores(logicalDevice);
        default:
            throw std::runtime_error("type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
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
            throw std::runtime_error("type_ == Type::SINGLE || type_ == Type::ARRAY");
    }
}
inline CommandBuffer::Type CommandBuffer::type() const
{
    return type_;
}
inline const commandBuffer::CommandBuffer &CommandBuffer::commandBuffer() const
{
    switch (type_)
    {
        case Type::SINGLE:
            return commandBuffer_;
        default:
            throw std::runtime_error("type_ == Type::SINGLE");
    }
}
inline const commandBuffer::CommandBufferArray &CommandBuffer::commandBufferArray() const
{
    switch (type_)
    {
        case Type::ARRAY:
            return commandBufferArray_;
        default:
            throw std::runtime_error("type_ == Type::ARRAY");
    }
}
} // namespace luna::core
