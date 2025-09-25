// Created by NBT22 on 2/12/25.
//

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/luna.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

static constexpr long double BLOCK_SIZE = 32 * 1024 * 1024;

namespace luna::helpers
{
static VkResult allocateBuffer(const LunaBufferCreationInfo &creationInfo)
{
    const VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = creationInfo.flags,
        .size = static_cast<VkDeviceSize>(BLOCK_SIZE * std::ceil(creationInfo.size / BLOCK_SIZE)),
        .usage = creationInfo.usage,
        .sharingMode = device.sharingMode(),
        .queueFamilyIndexCount = device.familyCount(),
        .pQueueFamilyIndices = device.queueFamilyIndices(),
    };
    TRY_CATCH_RESULT(luna::buffers.emplace_back(bufferCreateInfo));
    return VK_SUCCESS;
}
static bool sortBufferRegionsByOffsetAscending(const Buffer::BufferRegion &a, const Buffer::BufferRegion &b)
{
    return a.offset() < b.offset();
}
} // namespace luna::helpers

namespace luna
{
void BufferRegionIndex::destroyBuffer_(Buffer *buffer)
{
    buffer->shouldBeDestroyed_ = true;
    vkDeviceWaitIdle(device);
    buffer->canBeReused_ = false;
    if (!buffer->shouldBeDestroyed_)
    {
        buffer->canBeReused_ = true;
        return;
    }
    buffers.remove_if([&buffer](const Buffer &region) -> bool {
        return region.data_ == buffer->data_ &&
               region.allocation_ == buffer->allocation_ &&
               region.buffer_ == buffer->buffer_;
    });
}


Buffer::BufferRegion::BufferRegion(const size_t size, uint8_t *data, Buffer *buffer)
{
    assert(size_ == 0 || size <= size_);
    size_ = size;
    data_ = data;
    buffer_ = buffer;
}
Buffer::BufferRegion::BufferRegion(const size_t size,
                                   uint8_t *data,
                                   const size_t offset,
                                   Buffer *buffer,
                                   LunaBuffer *index):
    BufferRegion(size, data, buffer)
{
    offset_ = offset;
    bufferRegionIndices.emplace_back(buffer, this);
    if (index != nullptr)
    {
        *index = &bufferRegionIndices.back();
    }
}
Buffer::BufferRegion::BufferRegion(const size_t totalSize,
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

VkResult Buffer::BufferRegion::createBufferRegion(const LunaBufferCreationInfo &creationInfo,
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
            if (buffer->regions_.empty() || !buffer->canBeReused_)
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
            if (foundBuffer->shouldBeDestroyed_)
            {
                foundBuffer->shouldBeDestroyed_ = false;
            }
            if (foundBuffer->canBeReused_)
            {
                if (count > 1)
                {
                    assert(count > 1 && creationInfos);
                    foundBuffer->regions_.emplace_back(creationInfo.size,
                                                       static_cast<uint8_t *>(foundBuffer->data_) + baseOffset,
                                                       baseOffset,
                                                       foundBuffer,
                                                       count,
                                                       creationInfos,
                                                       bufferOut);
                } else
                {
                    foundBuffer->regions_.emplace_back(creationInfo.size,
                                                       static_cast<uint8_t *>(foundBuffer->data_) + baseOffset,
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
        CHECK_RESULT_RETURN(helpers::allocateBuffer(creationInfo));
        foundBuffer = &buffers.back();
    }
    const size_t offset = foundBuffer->usedBytes_ + foundBuffer->unusedBytes_;
    if (count > 1)
    {
        assert(count > 1 && creationInfos);
        foundBuffer->regions_.emplace_back(creationInfo.size,
                                           static_cast<uint8_t *>(foundBuffer->data_) + offset,
                                           offset,
                                           foundBuffer,
                                           count,
                                           creationInfos,
                                           bufferOut);
    } else
    {
        foundBuffer->regions_.emplace_back(creationInfo.size,
                                           static_cast<uint8_t *>(foundBuffer->data_) + offset,
                                           offset,
                                           foundBuffer,
                                           *bufferOut);
    }
    foundBuffer->freeBytes_ -= creationInfo.size;
    foundBuffer->usedBytes_ += creationInfo.size;
    return VK_SUCCESS;
}


Buffer::Buffer(const VkBufferCreateInfo &bufferCreateInfo)
{
    creationFlags_ = bufferCreateInfo.flags;
    usageFlags_ = bufferCreateInfo.usage;
    freeBytes_ = bufferCreateInfo.size;

    // TODO: Better memory types and allowing the application to pick based on VMA attributes
    VmaAllocationInfo allocationInfo;
    constexpr VmaAllocationCreateInfo allocationCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    CHECK_RESULT_THROW(vmaCreateBuffer(device.allocator(),
                                       &bufferCreateInfo,
                                       &allocationCreateInfo,
                                       &buffer_,
                                       &allocation_,
                                       &allocationInfo));
    data_ = allocationInfo.pMappedData;
}

Buffer::~Buffer()
{
    vmaDestroyBuffer(device.allocator(), buffer_, allocation_);
}


// static VkResult resizeBuffer(const LunaBuffer &lunaBuffer, const VkDeviceSize newSize)
// {
//     const Buffer &buffer = *static_cast<const Buffer *>(lunaBuffer);
// }
} // namespace luna

VkResult lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo)
{
    assert(creationInfo);
    return luna::helpers::allocateBuffer(*creationInfo);
}

VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer)
{
    assert(creationInfo);
    return luna::Buffer::BufferRegion::createBufferRegion(*creationInfo, &buffer);
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
    return luna::Buffer::BufferRegion::createBufferRegion(combinedCreationInfo, buffers, count, creationInfos);
}

void lunaDestroyBuffer(const LunaBuffer buffer)
{
    using namespace luna;
    assert(buffer);
    const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(buffer);
    bufferRegionIndices.remove_if([index](const BufferRegionIndex &regionIndex) -> bool {
        return regionIndex.buffer() == index->buffer() &&
               regionIndex.bufferRegion() == index->bufferRegion() &&
               regionIndex.subRegion() == index->subRegion();
    });
}

VkResult lunaResizeBuffer(const LunaBuffer buffer, const VkDeviceSize newSize)
{
    assert(buffer);

    // NOLINTNEXTLINE(*-pro-type-const-cast)
    return const_cast<luna::BufferRegionIndex *>(static_cast<const luna::BufferRegionIndex *>(buffer))->resize(newSize);
}

void lunaWriteDataToBuffer(const LunaBuffer buffer, const void *data, const size_t bytes, const size_t offset)
{
    if (bytes == 0)
    {
        return;
    }
    assert(buffer && data);
    const luna::BufferRegionIndex *bufferRegionIndex = static_cast<const luna::BufferRegionIndex *>(buffer);
    assert(bytes <= bufferRegionIndex->size() - offset);
    uint8_t *bufferData = bufferRegionIndex->data();
    std::copy_n(static_cast<const uint8_t *>(data), bytes, bufferData + offset);
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
        const luna::BufferRegionIndex *bufferRegionIndex = static_cast<const luna::BufferRegionIndex *>(buffers[i]);
        buffersVector.emplace_back(*bufferRegionIndex->buffer());
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
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, index->buffer(), &bufferOffset);
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
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, index->buffer(), &bufferOffset);
        }
    }
    vkCmdDrawIndirect(commandBuffer,
                      *static_cast<const BufferRegionIndex *>(buffer)->buffer(),
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
    const BufferRegionIndex *drawParameterBufferRegionIndex = static_cast<const BufferRegionIndex *>(buffer);
    const BufferRegionIndex *countBufferRegionIndex = static_cast<const BufferRegionIndex *>(countBuffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, index->buffer(), &bufferOffset);
        }
    }
    vkCmdDrawIndirectCount(commandBuffer,
                           *drawParameterBufferRegionIndex->buffer(),
                           offset + drawParameterBufferRegionIndex->offset(),
                           *countBufferRegionIndex->buffer(),
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
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, index->buffer(), &bufferOffset);
        }
    }
    if (indexBuffer != nullptr)
    {
        if (boundIndexBuffer != indexBuffer)
        {
            boundIndexBuffer = indexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundIndexBuffer);
            vkCmdBindIndexBuffer(commandBuffer, *index->buffer(), index->offset(), indexType);
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
    const BufferRegionIndex *bufferRegionIndex = static_cast<const BufferRegionIndex *>(buffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, index->buffer(), &bufferOffset);
        }
    }
    if (indexBuffer != nullptr)
    {
        if (boundIndexBuffer != indexBuffer)
        {
            boundIndexBuffer = indexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundIndexBuffer);
            vkCmdBindIndexBuffer(commandBuffer, *index->buffer(), index->offset(), indexType);
        }
    }
    vkCmdDrawIndexedIndirect(commandBuffer,
                             *bufferRegionIndex->buffer(),
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
    const BufferRegionIndex *drawParameterBufferRegionIndex = static_cast<const BufferRegionIndex *>(buffer);
    const BufferRegionIndex *countBufferRegionIndex = static_cast<const BufferRegionIndex *>(countBuffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        if (boundVertexBuffer != vertexBuffer)
        {
            boundVertexBuffer = vertexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundVertexBuffer);
            const size_t bufferOffset = index->offset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, index->buffer(), &bufferOffset);
        }
    }
    if (indexBuffer != nullptr)
    {
        if (boundIndexBuffer != indexBuffer)
        {
            boundIndexBuffer = indexBuffer;
            const BufferRegionIndex *index = static_cast<const BufferRegionIndex *>(boundIndexBuffer);
            vkCmdBindIndexBuffer(commandBuffer, *index->buffer(), index->offset(), indexType);
        }
    }
    vkCmdDrawIndexedIndirectCount(commandBuffer,
                                  *drawParameterBufferRegionIndex->buffer(),
                                  offset + drawParameterBufferRegionIndex->offset(),
                                  *countBufferRegionIndex->buffer(),
                                  countBufferOffset + countBufferRegionIndex->offset(),
                                  maxDrawCount,
                                  stride);
    return VK_SUCCESS;
}
