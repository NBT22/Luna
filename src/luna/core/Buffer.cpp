//
// Created by NBT22 on 2/12/25.
//

#include <algorithm>
#include <cmath>
#include <luna/core/Buffer.hpp>
#include <luna/core/Instance.hpp>
#include <luna/luna.h>

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
        .sharingMode = core::device.sharingMode(),
        .queueFamilyIndexCount = core::device.familyCount(),
        .pQueueFamilyIndices = core::device.queueFamilyIndices(),
    };
    TRY_CATCH_RESULT(core::buffers.emplace_back(bufferCreateInfo));
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna::core::buffer
{
VkResult BufferRegion::createBufferRegion(const LunaBufferCreationInfo &creationInfo,
                                          LunaBuffer **bufferOut,
                                          const uint32_t count,
                                          const LunaBufferCreationInfo *creationInfos)
{
    Buffer *foundBuffer = nullptr;
    std::vector<Buffer *> fallbackBuffers;
    for (Buffer &buffer: buffers)
    {
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
            const auto byAscendingOffset = [](const BufferRegion &a, const BufferRegion &b) -> bool {
                return a.offset() < b.offset();
            };
            if (!std::is_sorted(buffer->regions_.begin(), buffer->regions_.end(), byAscendingOffset))
            {
                buffer->regions_.sort(byAscendingOffset);
            }
            const auto hasLargeEnoughGap = [&creationInfo](const BufferRegion &a, const BufferRegion &b) -> bool {
                // TODO: Test to make sure this actually works
                return a.offset() + a.size() < b.offset() - creationInfo.size;
            };
            const std::list<BufferRegion>::const_iterator regionIterator = std::adjacent_find(buffer->regions_.begin(),
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

BufferRegion::BufferRegion(const size_t size, uint8_t *data, Buffer *buffer): size_(size), data_(data), buffer_(buffer)
{}
BufferRegion::BufferRegion(const size_t size, uint8_t *data, const size_t offset, Buffer *buffer, LunaBuffer *index)
{
    assert(isDestroyed_);
    assert(size_ == 0 || size <= size_);
    size_ = size;
    data_ = data;
    offset_ = offset;
    buffer_ = buffer;
    isDestroyed_ = false;
    bufferRegionIndices.emplace_back(buffer, this, nullptr);
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
                           LunaBuffer **buffers)
{
    assert(isDestroyed_);
    assert(size_ == 0 || totalSize <= size_);
    size_ = totalSize;
    data_ = data;
    offset_ = offset;
    buffer_ = buffer;

    uint32_t subRegionOffset = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        const size_t size = creationInfos[i].size;
        subRegions_.emplace_back(size, subRegionOffset);
        subRegionOffset += size;
        bufferRegionIndices.emplace_back(buffer, this, &subRegions_.back());
        if (buffers != nullptr && buffers[i] != nullptr)
        {
            *buffers[i] = &bufferRegionIndices.back();
        }
    }

    isDestroyed_ = false;
}

void BufferRegion::destroy()
{
    if (isDestroyed_)
    {
        return;
    }
    buffer_->unusedBytes_ += size_;
    buffer_->usedBytes_ -= size_;
    subRegions_.clear();
    isDestroyed_ = true;
}

void BufferRegion::destroyAtEnd()
{
    if (isDestroyed_)
    {
        return;
    }
    buffer_->freeBytes_ += size_;
    buffer_->usedBytes_ -= size_;
    size_ = 0;
    offset_ = 0;
    subRegions_.clear();
    isDestroyed_ = true;
}

void BufferRegion::destroySubRegion(const SubRegion *subRegion)
{
    const std::list<SubRegion>::iterator iterator = std::find_if(subRegions_.begin(),
                                                                 subRegions_.end(),
                                                                 [subRegion](const SubRegion &region) -> bool {
                                                                     return region.offset == subRegion->offset;
                                                                 });
    assert(iterator != subRegions_.end());
    if (subRegion->offset == 0)
    {
        buffer_->unusedBytes_ += subRegion->size;
        offset_ += subRegion->size;
        data_ += subRegion->size;
    } else
    {
        buffer_->freeBytes_ += subRegion->size;
    }
    buffer_->usedBytes_ -= subRegion->size;
    size_ -= subRegion->size;
    const std::list<SubRegion>::iterator endIterator = subRegions_.end();
    for (std::list<SubRegion>::iterator regionIterator = iterator; regionIterator != endIterator; ++regionIterator)
    {
        if (regionIterator->offset > subRegion->offset)
        {
            regionIterator->offset -= subRegion->size;
        }
    }
    subRegions_.erase(iterator);
    if (subRegions_.size() == 1)
    {
        assert(subRegions_.front().offset == 0 && subRegions_.front().size == size_);
        subRegions_.clear();
    }
}
} // namespace luna::core::buffer

namespace luna::core
{
Buffer::Buffer(const VkBufferCreateInfo &bufferCreateInfo)
{
    assert(isDestroyed_);
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
    isDestroyed_ = false;
}

void Buffer::destroy()
{
    if (isDestroyed_)
    {
        return;
    }
    vmaDestroyBuffer(device.allocator(), buffer_, allocation_);
    usedBytes_ = 0;
    regions_.clear();
    isDestroyed_ = true;
}
} // namespace luna::core

VkResult lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo)
{
    assert(creationInfo);
    return luna::helpers::allocateBuffer(*creationInfo);
}

VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer)
{
    assert(creationInfo);
    return luna::core::buffer::BufferRegion::createBufferRegion(*creationInfo, &buffer);
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
    return luna::core::buffer::BufferRegion::createBufferRegion(combinedCreationInfo, buffers, count, creationInfos);
}

void lunaDestroyBuffer(const LunaBuffer buffer)
{
    using namespace luna::core;
    assert(buffer);
    const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(buffer);
    if (index->subRegion == nullptr)
    {
        index->bufferRegion->destroy();
    } else
    {
        if (index->subRegion->offset == 0)
        {
            index->buffer->regions_.emplace_back(index->subRegion->size,
                                                 static_cast<uint8_t *>(index->buffer->data_) +
                                                         index->bufferRegion->offset(),
                                                 index->buffer);
        }
        index->bufferRegion->destroySubRegion(index->subRegion);
    }
    if (index->buffer->usedBytes_ == 0)
    {
        index->buffer->destroy();
    }
}

void lunaWriteDataToBuffer(const LunaBuffer buffer, const void *data, const size_t bytes, const size_t offset)
{
    if (bytes == 0)
    {
        return;
    }
    assert(buffer && data);
    const auto *bufferRegionIndex = static_cast<const luna::core::buffer::BufferRegionIndex *>(buffer);
    [[maybe_unused]] const size_t bufferSize = bufferRegionIndex->subRegion != nullptr
                                                       ? bufferRegionIndex->subRegion->size
                                                       : bufferRegionIndex->bufferRegion->size_;
    assert(bytes <= bufferSize - offset);
    uint8_t *bufferData = bufferRegionIndex->subRegion != nullptr
                                  ? bufferRegionIndex->bufferRegion->data_ + bufferRegionIndex->subRegion->offset
                                  : bufferRegionIndex->bufferRegion->data_;
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
        const auto *bufferRegionIndex = static_cast<const luna::core::buffer::BufferRegionIndex *>(buffers[i]);
        buffersVector.emplace_back(*bufferRegionIndex->buffer);
        offsetsVector[i] += bufferRegionIndex->bufferRegion->offset(bufferRegionIndex->subRegion);
    }
    vkCmdBindVertexBuffers(luna::core::device.commandPools().graphics.commandBuffer(),
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
    using namespace luna::core;
    assert(pipeline);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferOffset);
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
    using namespace luna::core;
    assert(pipeline && buffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferOffset);
    }
    vkCmdDrawIndirect(commandBuffer,
                      *static_cast<const buffer::BufferRegionIndex *>(buffer)->buffer,
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
    using namespace luna::core;
    assert(pipeline && buffer && countBuffer);
    const auto *drawParameterBufferRegionIndex = static_cast<const buffer::BufferRegionIndex *>(buffer);
    const auto *countBufferRegionIndex = static_cast<const buffer::BufferRegionIndex *>(countBuffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferOffset);
    }
    const size_t drawParameterBufferRegionOffset = drawParameterBufferRegionIndex->bufferRegion
                                                           ->offset(drawParameterBufferRegionIndex->subRegion);
    const size_t countBufferRegionOffset = countBufferRegionIndex->bufferRegion
                                                   ->offset(countBufferRegionIndex->subRegion);
    vkCmdDrawIndirectCount(commandBuffer,
                           *drawParameterBufferRegionIndex->buffer,
                           offset + drawParameterBufferRegionOffset,
                           *countBufferRegionIndex->buffer,
                           countBufferOffset + countBufferRegionOffset,
                           maxDrawCount,
                           stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexed(const LunaBuffer vertexBuffer,
                               const LunaBuffer indexBuffer,
                               const VkDeviceSize indexOffset,
                               const VkIndexType indexType,
                               const LunaGraphicsPipeline pipeline,
                               const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                               const uint32_t indexCount,
                               const uint32_t instanceCount,
                               const uint32_t firstIndex,
                               const int32_t vertexOffset,
                               const uint32_t firstInstance)
{
    using namespace luna::core;
    assert(pipeline);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferOffset);
    }
    if (indexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
        boundIndexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindIndexBuffer(commandBuffer, boundIndexBuffer, indexOffset + bufferOffset, indexType);
    }
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirect(const LunaBuffer vertexBuffer,
                                       const LunaBuffer indexBuffer,
                                       const VkDeviceSize indexOffset,
                                       const VkIndexType indexType,
                                       const LunaGraphicsPipeline pipeline,
                                       const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                       const LunaBuffer buffer,
                                       const VkDeviceSize offset,
                                       const uint32_t drawCount,
                                       const uint32_t stride)
{
    using namespace luna::core;
    assert(pipeline && buffer);
    const buffer::BufferRegionIndex *bufferRegionIndex = static_cast<const buffer::BufferRegionIndex *>(buffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferOffset);
    }
    if (indexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
        boundIndexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindIndexBuffer(commandBuffer, boundIndexBuffer, indexOffset + bufferOffset, indexType);
    }
    const size_t bufferRegionOffset = bufferRegionIndex->bufferRegion->offset(bufferRegionIndex->subRegion);
    vkCmdDrawIndexedIndirect(commandBuffer, *bufferRegionIndex->buffer, offset + bufferRegionOffset, drawCount, stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirectCount(const LunaBuffer vertexBuffer,
                                            const LunaBuffer indexBuffer,
                                            const VkDeviceSize indexOffset,
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
    using namespace luna::core;
    assert(pipeline && buffer && countBuffer);
    const auto *drawParameterBufferRegionIndex = static_cast<const buffer::BufferRegionIndex *>(buffer);
    const auto *countBufferRegionIndex = static_cast<const buffer::BufferRegionIndex *>(countBuffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(static_cast<const GraphicsPipeline *>(pipeline)->bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferOffset);
    }
    if (indexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
        boundIndexBuffer = *index->buffer;
        const size_t bufferOffset = index->bufferRegion->offset(index->subRegion);
        vkCmdBindIndexBuffer(commandBuffer, boundIndexBuffer, indexOffset + bufferOffset, indexType);
    }
    const size_t drawParameterBufferRegionOffset = drawParameterBufferRegionIndex->bufferRegion
                                                           ->offset(drawParameterBufferRegionIndex->subRegion);
    const size_t countBufferRegionOffset = countBufferRegionIndex->bufferRegion
                                                   ->offset(countBufferRegionIndex->subRegion);
    vkCmdDrawIndexedIndirectCount(commandBuffer,
                                  *drawParameterBufferRegionIndex->buffer,
                                  offset + drawParameterBufferRegionOffset,
                                  *countBufferRegionIndex->buffer,
                                  countBufferOffset + countBufferRegionOffset,
                                  maxDrawCount,
                                  stride);
    return VK_SUCCESS;
}
