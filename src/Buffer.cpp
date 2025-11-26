// Created by NBT22 on 2/12/25.
//

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/lunaBuffer.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "helpers/Handle.hpp"
#include "Instance.hpp"
#include "Luna.hpp"
#include "Semaphore.hpp"

static constexpr long double BLOCK_SIZE = 32 * 1024 * 1024;

namespace luna::helpers
{
static bool sortBufferRegionsByOffsetAscending(const BufferRegion &a, const BufferRegion &b)
{
    return a.offset() < b.offset();
}
} // namespace luna::helpers

namespace luna
{
BufferRegion::BufferRegion(const size_t size, uint8_t *data, Buffer *buffer)
{
    assert(size_ == 0 || size <= size_);
    size_ = size;
    data_ = data;
    buffer_ = buffer;
}
BufferRegion::BufferRegion(const size_t size, uint8_t *data, const size_t offset, Buffer *buffer, LunaBuffer *index):
    BufferRegion(size, data, buffer)
{
    offset_ = offset;
    bufferRegionIndices.emplace_back(buffer, this);
    if (index != nullptr)
    {
        *index = &bufferRegionIndices.back();
    }
}
BufferRegion::BufferRegion(const size_t totalSize,
                           uint8_t *data,
                           const size_t offset,
                           Buffer *buffer,
                           const uint32_t count,
                           const LunaBufferCreationInfo *creationInfos,
                           LunaBuffer **lunaBuffers):
    BufferRegion(totalSize, data, buffer)
{
    offset_ = offset;

    uint32_t subRegionOffset = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        const size_t size = creationInfos[i].size;
        subRegions_.emplace_back(size, subRegionOffset);
        subRegionOffset += size;
        bufferRegionIndices.emplace_back(buffer, this, &subRegions_.back());
        if (lunaBuffers != nullptr && lunaBuffers[i] != nullptr)
        {
            *lunaBuffers[i] = &bufferRegionIndices.back();
        }
    }
}

VkResult BufferRegion::createBufferRegion(const LunaBufferCreationInfo &creationInfo, LunaBuffer *buffer)
{
    return createBufferRegion(creationInfo, &buffer, 1, nullptr);
}

VkResult BufferRegion::createBufferRegion(const LunaBufferCreationInfo &creationInfo,
                                          LunaBuffer **bufferOut,
                                          uint32_t count,
                                          const LunaBufferCreationInfo *creationInfos)

{
    Buffer *foundBuffer = nullptr;
    std::vector<Buffer *> fallbackBuffers;
    for (Buffer &buffer: buffers)
    {
        if ((buffer.usageFlags_ & creationInfo.usage) != creationInfo.usage ||
            (buffer.creationFlags_ & creationInfo.flags) != creationInfo.flags)
        {
            continue;
        }
        if (creationInfo.size <= buffer.unusedBytes_)
        {
            fallbackBuffers.clear();
            foundBuffer = &buffer;
            break;
        }
        if (creationInfo.size <= buffer.freeBytes_)
        {
            fallbackBuffers.emplace_back(&buffer);
        }
    }
    if (!fallbackBuffers.empty())
    {
        size_t baseOffset = 0;
        const auto hasFreeSpace = [&creationInfo, &baseOffset](Buffer *&buffer) -> bool {
            if (buffer->regions_.empty() || buffer->destroyed_)
            {
                return false;
            }
            if (buffer->regions_.size() == 1)
            {
                return creationInfo.size <= buffer->regions_.front().offset_;
            }
            if (!std::is_sorted(buffer->regions_.begin(),
                                buffer->regions_.end(),
                                helpers::sortBufferRegionsByOffsetAscending))
            {
                buffer->regions_.sort(helpers::sortBufferRegionsByOffsetAscending);
            }
            const auto hasLargeEnoughGap = [&creationInfo](const BufferRegion &a, const BufferRegion &b) -> bool {
                return a.offset() + a.size() < b.offset() - creationInfo.size;
            };
            const std::list<BufferRegion>::iterator regionIterator = std::adjacent_find(buffer->regions_.begin(),
                                                                                        buffer->regions_.end(),
                                                                                        hasLargeEnoughGap);
            if (regionIterator == buffer->regions_.end())
            {
                return false;
            }
            baseOffset = regionIterator->offset();
            return true;
        };
        const std::vector<Buffer *>::iterator bufferIterator = std::find_if(fallbackBuffers.begin(),
                                                                            fallbackBuffers.end(),
                                                                            hasFreeSpace);

        if (bufferIterator != fallbackBuffers.end())
        {
            foundBuffer = *bufferIterator;
            if (!foundBuffer->destroyed_)
            {
                uint8_t *regionData = foundBuffer->data_ == nullptr
                                              ? nullptr
                                              : static_cast<uint8_t *>(foundBuffer->data_) + baseOffset;
                if (count > 1)
                {
                    assert(count > 1 && creationInfos);
                    foundBuffer->regions_.emplace_back(creationInfo.size,
                                                       regionData,
                                                       baseOffset,
                                                       foundBuffer,
                                                       count,
                                                       creationInfos,
                                                       bufferOut);
                } else
                {
                    foundBuffer->regions_.emplace_back(creationInfo.size,
                                                       regionData,
                                                       baseOffset,
                                                       foundBuffer,
                                                       *bufferOut);
                }
                foundBuffer->unusedBytes_ -= creationInfo.size;
                foundBuffer->usedBytes_ += creationInfo.size;
                return VK_SUCCESS;
            }
        }
        foundBuffer = nullptr;
    }
    if (foundBuffer == nullptr)
    {
        const VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = creationInfo.flags,
            .size = static_cast<VkDeviceSize>(BLOCK_SIZE * std::ceil(creationInfo.size / BLOCK_SIZE)),
            .usage = creationInfo.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = device.sharingMode(),
            .queueFamilyIndexCount = device.familyCount(),
            .pQueueFamilyIndices = device.queueFamilyIndices(),
        };
        constexpr VmaAllocationCreateInfo allocationCreateInfo = {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        };
        TRY_CATCH_RESULT(luna::buffers.emplace_back(bufferCreateInfo,
                                                    creationInfo.allocationCreateInfo
                                                            ? *creationInfo.allocationCreateInfo
                                                            : allocationCreateInfo));
        foundBuffer = &buffers.back();
    }
    const size_t offset = foundBuffer->usedBytes_ + foundBuffer->unusedBytes_;
    uint8_t *regionData = foundBuffer->data_ == nullptr ? nullptr : static_cast<uint8_t *>(foundBuffer->data_) + offset;
    if (count > 1)
    {
        assert(count > 1 && creationInfos);
        foundBuffer->regions_.emplace_back(creationInfo.size,
                                           regionData,
                                           offset,
                                           foundBuffer,
                                           count,
                                           creationInfos,
                                           bufferOut);
    } else
    {
        foundBuffer->regions_.emplace_back(creationInfo.size, regionData, offset, foundBuffer, *bufferOut);
    }
    foundBuffer->freeBytes_ -= creationInfo.size;
    foundBuffer->usedBytes_ += creationInfo.size;
    return VK_SUCCESS;
}


VkResult BufferRegionIndex::copyToBuffer(const uint8_t *data,
                                         const size_t bytes,
                                         const size_t offset,
                                         const VkPipelineStageFlags stageFlags) const
{
    assert(bytes <= size() - offset);
    uint8_t *mappedData = BufferRegionIndex::data();
    if (mappedData != nullptr)
    {
        std::copy_n(data, bytes, mappedData + offset);
    } else
    {
        // TODO: Should this use a dedicated transfer command buffer
        CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer(1);
        CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(luna::device, true));
        CHECK_RESULT_RETURN(reserve(stagingBuffer, bytes));
        assert(stagingBuffer->data() != nullptr);
        // ReSharper disable once CppDFANullDereference
        std::copy_n(data, bytes, stagingBuffer->data() + offset);
        const VkBufferCopy copyRegion = {
            .srcOffset = stagingBuffer->offset(),
            .dstOffset = BufferRegionIndex::offset() + offset,
            .size = bytes,
        };
        vkCmdCopyBuffer(commandBuffer, stagingBuffer->buffer(), buffer(), 1, &copyRegion);
        const Semaphore &semaphore = commandBuffer.semaphore();
        const VkSubmitInfo queueSubmitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = semaphore.isSignaled() ? 1u : 0u,
            .pWaitSemaphores = semaphore.isSignaled() ? &semaphore : nullptr,
            .pWaitDstStageMask = &semaphore.stageMask(),
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &semaphore,
        };
        CHECK_RESULT_RETURN(commandBuffer.submitCommandBuffer(luna::device.familyQueues().graphics,
                                                              queueSubmitInfo,
                                                              stageFlags));
    }
    return VK_SUCCESS;
}

void BufferRegionIndex::creationInfo(LunaBufferCreationInfo *creationInfo) const
{
    creationInfo->size = size();
    buffer_->creationInfo(creationInfo);
}

BufferRegionIndex::~BufferRegionIndex()
{
    assert(buffer_ && bufferRegion_);
    if (subRegion_ != nullptr)
    {
        const std::list<SubRegion>::iterator endIterator = bufferRegion_->subRegions_.end();
        const std::list<SubRegion>::iterator iterator = std::find_if(bufferRegion_->subRegions_.begin(),
                                                                     endIterator,
                                                                     [this](const SubRegion &region) -> bool {
                                                                         return region.offset == subRegion_->offset;
                                                                     });
        assert(iterator != endIterator);
        buffer_->freeBytes_ += subRegion_->size;
        buffer_->usedBytes_ -= subRegion_->size;
        bufferRegion_->size_ -= subRegion_->size;
        if (subRegion_->offset == 0)
        {
            bufferRegion_->offset_ += subRegion_->size;
            if (bufferRegion_->data_ != nullptr)
            {
                bufferRegion_->data_ += subRegion_->size;
            }
            for (std::list<SubRegion>::iterator regionIterator = iterator; regionIterator != endIterator;
                 ++regionIterator)
            {
                if (regionIterator->offset > subRegion_->offset)
                {
                    regionIterator->offset -= subRegion_->size;
                }
            }
        }
        bufferRegion_->subRegions_.erase(iterator);
    }
    if (subRegion_ == nullptr || bufferRegion_->subRegions_.empty())
    {
        assert(bufferRegion_->subRegions_.empty());
        buffer_->regions_.remove_if([this](const BufferRegion &region) -> bool {
            return region.offset_ == bufferRegion_->offset_;
        });
    }
    if (buffer_->regions_.empty())
    {
        if (cleanupThread_.joinable())
        {
            cleanupThread_.join();
        }
        cleanupThread_ = std::thread(
                [](const Buffer *buffer) -> void {
                    buffers.remove_if([&buffer](const Buffer &other) -> bool { return *buffer == other; });
                },
                buffer_);
    }
}


Buffer::Buffer(const VkBufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo)
{
    BufferRegionIndex::waitForCleanupThread();

    creationFlags_ = bufferCreateInfo.flags;
    usageFlags_ = bufferCreateInfo.usage;
    freeBytes_ = bufferCreateInfo.size;
    allocationCreateInfo_ = allocationCreateInfo;

    VmaAllocationInfo allocationInfo;
    CHECK_RESULT_THROW(vmaCreateBuffer(device.allocator(),
                                       &bufferCreateInfo,
                                       &allocationCreateInfo,
                                       &buffer_,
                                       &allocation_,
                                       &allocationInfo));
    data_ = allocationInfo.pMappedData;
    destroyed_ = false;
}

Buffer::~Buffer()
{
    destroyed_ = true;
    vkDeviceWaitIdle(device); // TODO: This is a terrible solution
    vmaDestroyBuffer(device.allocator(), buffer_, allocation_);
}
} // namespace luna

VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer)
{
    assert(creationInfo);
    return luna::BufferRegion::createBufferRegion(*creationInfo, buffer);
}

VkResult lunaCreateBuffers(const uint32_t count, const LunaBufferCreationInfo *creationInfos, LunaBuffer **buffers)
{
    assert(creationInfos);
    LunaBufferCreationInfo combinedCreationInfo{};
    for (uint32_t i = 0; i < count; i++)
    {
        const LunaBufferCreationInfo &creationInfo = creationInfos[i];
        combinedCreationInfo.size += creationInfo.size;
        combinedCreationInfo.flags |= creationInfo.flags;
        combinedCreationInfo.usage |= creationInfo.usage;
    }
    return luna::BufferRegion::createBufferRegion(combinedCreationInfo, buffers, count, creationInfos);
}

void lunaDestroyBuffer(const LunaBuffer buffer)
{
    assert(buffer);
    const luna::BufferRegionIndex &index = *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    luna::bufferRegionIndices.remove_if([&index](const luna::BufferRegionIndex &regionIndex) -> bool {
        return regionIndex == index;
    });
}

VkResult lunaResizeBuffer(const LunaBuffer *buffer, const VkDeviceSize newSize)
{
    assert(*buffer);
    luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(*buffer);
    return luna::BufferRegionIndex::resize(bufferRegionIndex, newSize);
}

VkResult lunaWriteDataToBuffer(const LunaBuffer buffer, const LunaBufferWriteInfo *writeInfo)
{
    assert(writeInfo != nullptr);
    if (writeInfo->bytes != 0)
    {
        assert(buffer != nullptr);
        assert(writeInfo->data != nullptr);

        const luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
        CHECK_RESULT_RETURN(bufferRegionIndex->copyToBuffer(static_cast<const uint8_t *>(writeInfo->data),
                                                            writeInfo->bytes,
                                                            writeInfo->offset,
                                                            writeInfo->stageFlags));
    }
    return VK_SUCCESS;
}

void lunaBufferGetCreationInfo(const LunaBuffer buffer, LunaBufferCreationInfo *creationInfo)
{
    luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer)->creationInfo(creationInfo);
}

void lunaBindVertexBuffers(const uint32_t firstBinding,
                           const uint32_t bindingCount,
                           const LunaBuffer *buffers,
                           const VkDeviceSize *offsets)
{
    std::vector<VkBuffer> buffersVector;
    buffersVector.reserve(bindingCount);
    std::vector<VkDeviceSize> offsetsVector(offsets, offsets + bindingCount);
    for (uint32_t i = 0; i < bindingCount; i++)
    {
        const luna::BufferRegionIndex *bufferRegionIndex =
                luna::helpers::fromHandle<luna::BufferRegionIndex>(buffers[i]);
        buffersVector.emplace_back(bufferRegionIndex->buffer());
        offsetsVector[i] += bufferRegionIndex->offset();
    }
    vkCmdBindVertexBuffers(luna::device.commandPools().graphics.commandBuffer(),
                           firstBinding,
                           bindingCount,
                           buffersVector.data(),
                           offsetsVector.data());
}

VkResult lunaDrawBuffer(const LunaBuffer vertexBuffer,
                        const LunaGraphicsPipeline pipeline,
                        const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                        const uint32_t vertexCount,
                        const uint32_t instanceCount,
                        const uint32_t firstVertex,
                        const uint32_t firstInstance)
{
    using namespace luna;
    assert(pipeline);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<GraphicsPipeline>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
        }
    }
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndirect(const LunaBuffer vertexBuffer,
                                const LunaGraphicsPipeline pipeline,
                                const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                const LunaBuffer buffer,
                                const VkDeviceSize offset,
                                const uint32_t drawCount,
                                const uint32_t stride)
{
    using namespace luna;
    assert(pipeline && buffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<GraphicsPipeline>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
        }
    }
    vkCmdDrawIndirect(commandBuffer,
                      luna::helpers::fromHandle<BufferRegionIndex>(buffer)->buffer(),
                      offset,
                      drawCount,
                      stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndirectCount(const LunaBuffer vertexBuffer,
                                     const LunaGraphicsPipeline pipeline,
                                     const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                     const LunaBuffer buffer,
                                     const VkDeviceSize offset,
                                     const LunaBuffer countBuffer,
                                     const VkDeviceSize countBufferOffset,
                                     const uint32_t maxDrawCount,
                                     const uint32_t stride)
{
    using namespace luna;
    assert(pipeline && buffer && countBuffer);
    const BufferRegionIndex *drawParameterBufferRegionIndex = luna::helpers::fromHandle<BufferRegionIndex>(buffer);
    const BufferRegionIndex *countBufferRegionIndex = luna::helpers::fromHandle<BufferRegionIndex>(countBuffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<GraphicsPipeline>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
        }
    }
    vkCmdDrawIndirectCount(commandBuffer,
                           drawParameterBufferRegionIndex->buffer(),
                           offset + drawParameterBufferRegionIndex->offset(),
                           countBufferRegionIndex->buffer(),
                           countBufferOffset + countBufferRegionIndex->offset(),
                           maxDrawCount,
                           stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexed(const LunaBuffer vertexBuffer,
                               const LunaBuffer indexBuffer,
                               const VkIndexType indexType,
                               const LunaGraphicsPipeline pipeline,
                               const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                               const uint32_t indexCount,
                               const uint32_t instanceCount,
                               const uint32_t firstIndex,
                               const int32_t vertexOffset,
                               const uint32_t firstInstance)
{
    using namespace luna;
    assert(pipeline);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<GraphicsPipeline>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
        }
    }
    if (indexBuffer != nullptr)
    {
        if (boundIndexBuffer != indexBuffer)
        {
            boundIndexBuffer = indexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundIndexBuffer);
            vkCmdBindIndexBuffer(commandBuffer, index->buffer(), index->offset(), indexType);
        }
    }
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirect(const LunaBuffer vertexBuffer,
                                       const LunaBuffer indexBuffer,
                                       const VkIndexType indexType,
                                       const LunaGraphicsPipeline pipeline,
                                       const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                       const LunaBuffer buffer,
                                       const VkDeviceSize offset,
                                       const uint32_t drawCount,
                                       const uint32_t stride)
{
    using namespace luna;
    assert(pipeline && buffer);
    const BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<BufferRegionIndex>(buffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<GraphicsPipeline>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
        }
    }
    if (indexBuffer != nullptr)
    {
        if (boundIndexBuffer != indexBuffer)
        {
            boundIndexBuffer = indexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundIndexBuffer);
            vkCmdBindIndexBuffer(commandBuffer, index->buffer(), index->offset(), indexType);
        }
    }
    vkCmdDrawIndexedIndirect(commandBuffer,
                             bufferRegionIndex->buffer(),
                             bufferRegionIndex->offset() + offset,
                             drawCount,
                             stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirectCount(const LunaBuffer vertexBuffer,
                                            const LunaBuffer indexBuffer,
                                            const VkIndexType indexType,
                                            const LunaGraphicsPipeline pipeline,
                                            const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                            const LunaBuffer buffer,
                                            const VkDeviceSize offset,
                                            const LunaBuffer countBuffer,
                                            const VkDeviceSize countBufferOffset,
                                            const uint32_t maxDrawCount,
                                            const uint32_t stride)
{
    using namespace luna;
    assert(pipeline && buffer && countBuffer);
    const BufferRegionIndex *drawParameterBufferRegionIndex = luna::helpers::fromHandle<BufferRegionIndex>(buffer);
    const BufferRegionIndex *countBufferRegionIndex = luna::helpers::fromHandle<BufferRegionIndex>(countBuffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<GraphicsPipeline>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
        }
    }
    if (indexBuffer != nullptr)
    {
        if (boundIndexBuffer != indexBuffer)
        {
            boundIndexBuffer = indexBuffer;
            const BufferRegionIndex *index = luna::helpers::fromHandle<BufferRegionIndex>(boundIndexBuffer);
            vkCmdBindIndexBuffer(commandBuffer, index->buffer(), index->offset(), indexType);
        }
    }
    vkCmdDrawIndexedIndirectCount(commandBuffer,
                                  drawParameterBufferRegionIndex->buffer(),
                                  offset + drawParameterBufferRegionIndex->offset(),
                                  countBufferRegionIndex->buffer(),
                                  countBufferOffset + countBufferRegionIndex->offset(),
                                  maxDrawCount,
                                  stride);
    return VK_SUCCESS;
}
