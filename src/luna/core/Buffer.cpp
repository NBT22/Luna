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
static VkResult allocateBuffer(const LunaBufferCreationInfo &creationInfo,
                               std::vector<core::Buffer>::iterator *iterator)
{
    core::buffers.reserve(core::buffers.size() + 1);
    const std::vector<core::Buffer>::iterator &bufferIterator = std::find_if(core::buffers.begin(),
                                                                             core::buffers.end(),
                                                                             core::Buffer::isDestroyed);
    const VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = creationInfo.flags,
        .size = static_cast<VkDeviceSize>(BLOCK_SIZE * std::ceil(creationInfo.size / BLOCK_SIZE)),
        .usage = creationInfo.usage,
        .sharingMode = core::device.sharingMode(),
        .queueFamilyIndexCount = core::device.familyCount(),
        .pQueueFamilyIndices = core::device.queueFamilyIndices(),
    };
    if (bufferIterator == core::buffers.end())
    {
        TRY_CATCH_RESULT(core::buffers.emplace_back(bufferCreateInfo));
    } else
    {
        TRY_CATCH_RESULT(*bufferIterator.base() = core::Buffer(bufferCreateInfo));
    }

    if (iterator != nullptr)
    {
        *iterator = bufferIterator;
    }
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna::core::buffer
{
VkResult BufferRegion::createBufferRegion(const LunaBufferCreationInfo &creationInfo, LunaBuffer *index)
{
    std::vector<Buffer>::iterator bufferIterator;
    BufferRegion *bufferRegion = nullptr;
    uint32_t regionIndex;
    CHECK_RESULT_RETURN(Buffer::findSpaceForRegion(creationInfo, bufferIterator, bufferRegion, regionIndex));
    const uint32_t bufferIndex = bufferIterator - buffers.begin();

    const size_t offset = bufferRegion == nullptr || bufferRegion->size_ == 0
                                  ? bufferIterator->usedBytes_ + bufferIterator->unusedBytes_
                                  : bufferRegion->offset_;
    if (bufferRegion == nullptr)
    {
        regionIndex = bufferIterator->regions_.size();
        bufferIterator->regions_.emplace_back(creationInfo.size,
                                              static_cast<uint8_t *>(bufferIterator->data_) + offset,
                                              offset,
                                              bufferIndex,
                                              regionIndex,
                                              index);
    } else
    {
        assert(bufferRegion->isDestroyed_);
        assert(bufferRegion->size_ == 0 || creationInfo.size <= bufferRegion->size_);
        bufferRegion->size_ = creationInfo.size;
        bufferRegion->data_ = static_cast<uint8_t *>(bufferIterator->data_) + offset;
        bufferRegion->offset_ = offset;
        bufferRegion->bufferIndex_ = bufferIndex;
        bufferRegionIndices.emplace_back(bufferIndex, regionIndex, nullptr);
        if (index != nullptr)
        {
            *index = &bufferRegionIndices.back();
        }
    }
    bufferIterator->usedBytes_ += creationInfo.size;
    bufferIterator->freeBytes_ -= creationInfo.size;

    return VK_SUCCESS;
}
VkResult BufferRegion::createBufferRegions(const uint32_t count,
                                           const LunaBufferCreationInfo *creationInfos,
                                           LunaBuffer *buffers)
{
    LunaBufferCreationInfo combinedCreationInfo{};
    for (uint32_t i = 0; i < count; i++)
    {
        const LunaBufferCreationInfo &creationInfo = creationInfos[i];
        combinedCreationInfo.size += creationInfo.size;
        combinedCreationInfo.flags |= creationInfo.flags;
        combinedCreationInfo.usage |= creationInfo.usage;
    }
    std::vector<Buffer>::iterator bufferIterator;
    BufferRegion *bufferRegion = nullptr;
    uint32_t regionIndex;
    CHECK_RESULT_RETURN(Buffer::findSpaceForRegion(combinedCreationInfo, bufferIterator, bufferRegion, regionIndex));
    const uint32_t bufferIndex = bufferIterator - core::buffers.begin();

    const size_t offset = bufferRegion == nullptr || bufferRegion->size_ == 0 ? bufferIterator->usedBytes_
                                                                              : bufferRegion->offset_;
    if (bufferRegion == nullptr)
    {
        bufferIterator->regions_.emplace_back(combinedCreationInfo.size,
                                              static_cast<uint8_t *>(bufferIterator->data_) + offset,
                                              offset,
                                              bufferIndex,
                                              regionIndex,
                                              count,
                                              creationInfos,
                                              buffers);
    } else
    {
        assert(bufferRegion->isDestroyed_);
        assert(bufferRegion->size_ == 0 || combinedCreationInfo.size <= bufferRegion->size_);
        bufferRegion->size_ = combinedCreationInfo.size;
        bufferRegion->data_ = static_cast<uint8_t *>(bufferIterator->data_) + offset;
        bufferRegion->offset_ = offset;
        bufferRegion->bufferIndex_ = bufferIndex;
        uint32_t subRegionOffset = 0;
        for (uint32_t i = 0; i < count; i++)
        {
            const size_t size = creationInfos[i].size;
            bufferRegion->subRegions_.emplace_back(size, subRegionOffset);
            subRegionOffset += size;
            bufferRegionIndices.emplace_back(bufferIndex, regionIndex, &bufferRegion->subRegions_.back());
            if (buffers != nullptr && buffers[i] != nullptr)
            {
                buffers[i] = &bufferRegionIndices.back();
            }
        }
    }
    bufferIterator->usedBytes_ += combinedCreationInfo.size;
    bufferIterator->freeBytes_ -= combinedCreationInfo.size;
    return VK_SUCCESS;
}

BufferRegion::BufferRegion(const size_t size,
                           uint8_t *data,
                           const size_t offset,
                           const uint32_t bufferIndex,
                           const uint32_t regionIndex,
                           LunaBuffer *index)
{
    assert(isDestroyed_);
    assert(size_ == 0 || size <= size_);
    size_ = size;
    data_ = data;
    offset_ = offset;
    bufferIndex_ = bufferIndex;
    isDestroyed_ = false;
    bufferRegionIndices.emplace_back(bufferIndex, regionIndex, nullptr);
    if (index != nullptr)
    {
        *index = &bufferRegionIndices.back();
    }
}
BufferRegion::BufferRegion(const size_t totalSize,
                           uint8_t *data,
                           const size_t offset,
                           const uint32_t bufferIndex,
                           const uint32_t regionIndex,
                           const uint32_t count,
                           const LunaBufferCreationInfo *creationInfos,
                           LunaBuffer *buffers)
{
    assert(isDestroyed_);
    assert(size_ == 0 || totalSize <= size_);
    size_ = totalSize;
    data_ = data;
    offset_ = offset;
    bufferIndex_ = bufferIndex;

    uint32_t subRegionOffset = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        const size_t size = creationInfos[i].size;
        subRegions_.emplace_back(size, subRegionOffset);
        subRegionOffset += size;
        bufferRegionIndices.emplace_back(bufferIndex, regionIndex, &subRegions_.back());
        if (buffers != nullptr && buffers[i] != nullptr)
        {
            buffers[i] = &bufferRegionIndices.back();
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
    Buffer &buffer = buffers.at(bufferIndex_);
    buffer.unusedBytes_ += size_;
    buffer.usedBytes_ -= size_;
    subRegions_.clear();
    isDestroyed_ = true;
}

void BufferRegion::destroyAtEnd()
{
    if (isDestroyed_)
    {
        return;
    }
    Buffer &buffer = buffers.at(bufferIndex_);
    buffer.freeBytes_ += size_;
    buffer.usedBytes_ -= size_;
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
    const auto &[size, offset] = *subRegion;
    Buffer &buffer = buffers.at(bufferIndex_);
    buffer.freeBytes_ += size;
    buffer.usedBytes_ -= size;
    size_ -= size;
    const std::list<SubRegion>::iterator endIterator = subRegions_.end();
    for (std::list<SubRegion>::iterator regionIterator = iterator; regionIterator != endIterator; ++regionIterator)
    {
        if (regionIterator->offset > offset)
        {
            regionIterator->size -= size;
        }
    }
    subRegions_.erase(iterator);
}
} // namespace luna::core::buffer

namespace luna::core
{
VkResult Buffer::findSpaceForRegion(const LunaBufferCreationInfo &creationInfo,
                                    std::vector<Buffer>::iterator &bufferIterator,
                                    buffer::BufferRegion *&bufferRegion,
                                    uint32_t &regionIndex)
{
    const auto hasUnusedRegion = [&creationInfo, &regionIndex](const Buffer &buffer) -> bool {
        if (!buffer.isDestroyed_ &&
            creationInfo.size <= buffer.unusedBytes_ &&
            (creationInfo.flags & buffer.creationFlags_) == creationInfo.flags &&
            (creationInfo.usage & buffer.usageFlags_) == creationInfo.usage)
        {
            const auto regionSizeEqual = [&creationInfo](const buffer::BufferRegion &region) -> bool {
                return buffer::BufferRegion::isDestroyed(region) && creationInfo.size == region.size();
            };
            std::vector<buffer::BufferRegion>::const_iterator regionIterator = std::find_if(buffer.regions_.cbegin(),
                                                                                            buffer.regions_.cend(),
                                                                                            regionSizeEqual);
            if (regionIterator != buffer.regions_.cend())
            {
                regionIndex = regionIterator - buffer.regions_.cbegin();
                return true;
            }
            regionIterator = std::find_if(buffer.regions_.cbegin(),
                                          buffer.regions_.cend(),
                                          [&creationInfo](const buffer::BufferRegion &region) -> bool {
                                              return buffer::BufferRegion::isDestroyed(region) &&
                                                     creationInfo.size < region.size();
                                          });
            if (regionIterator != buffer.regions_.cend())
            {
                regionIndex = regionIterator - buffer.regions_.cbegin();
                return true;
            }
        }
        return false;
    };
    bufferIterator = std::find_if(buffers.begin(), buffers.end(), hasUnusedRegion);
    if (bufferIterator == buffers.end())
    {
        const auto hasFreeSpace = [&creationInfo](const Buffer &buffer) -> bool {
            return !buffer.isDestroyed_ &&
                   creationInfo.size <= buffer.freeBytes_ &&
                   (creationInfo.flags & buffer.creationFlags_) == creationInfo.flags &&
                   (creationInfo.usage & buffer.usageFlags_) == creationInfo.usage;
        };
        bufferIterator = std::find_if(buffers.begin(), buffers.end(), hasFreeSpace);
        if (bufferIterator == buffers.end())
        {
            CHECK_RESULT_RETURN(helpers::allocateBuffer(creationInfo, &bufferIterator));
        }
        bufferRegion = nullptr;
    }
    return VK_SUCCESS;
}

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
    regions_.shrink_to_fit();
    isDestroyed_ = true;
}
} // namespace luna::core

VkResult lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo)
{
    assert(creationInfo);
    return luna::helpers::allocateBuffer(*creationInfo, nullptr);
}

VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer)
{
    assert(creationInfo);
    return luna::core::buffer::BufferRegion::createBufferRegion(*creationInfo, buffer);
}

// TODO: Actually implement this
VkResult lunaCreateBuffers(const uint32_t count, const LunaBufferCreationInfo *creationInfos, LunaBuffer *buffers)
{
    assert(creationInfos);
    return luna::core::buffer::BufferRegion::createBufferRegions(count, creationInfos, buffers);
}

void lunaDestroyBuffer(const LunaBuffer buffer)
{
    using namespace luna::core;
    assert(buffer);
    const buffer::BufferRegionIndex &index = *static_cast<const buffer::BufferRegionIndex *>(buffer);
    Buffer &bufferObject = buffers.at(index.bufferIndex);
    if (index.subRegion == nullptr)
    {
        bufferObject.destroyBufferRegion(index.bufferRegionIndex);
    } else
    {
        bufferObject.destroyBufferRegionSubRegion(index.bufferRegionIndex, index.subRegion);
    }
    if (bufferObject.usedBytes_ == 0)
    {
        bufferObject.destroy();
    }
}

void lunaWriteDataToBuffer(const LunaBuffer buffer, const void *data, const size_t bytes)
{
    if (bytes == 0)
    {
        return;
    }
    assert(data);
    std::copy_n(static_cast<const uint8_t *>(data), bytes, luna::core::bufferRegion(buffer).data_);
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
        buffersVector.emplace_back(luna::core::buffers.at(bufferRegionIndex->bufferIndex));
        offsetsVector[i] += luna::core::bufferRegion(buffers[i]).offset();
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
    CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
                                .bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferRegion(*index).offset());
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
    const uint32_t drawParameterBufferIndex = static_cast<const buffer::BufferRegionIndex *>(buffer)->bufferIndex;
    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
                                .bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferRegion(*index).offset());
    }
    vkCmdDrawIndirect(commandBuffer, buffers.at(drawParameterBufferIndex), offset, drawCount, stride);
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
    CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
                                .bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferRegion(*index).offset());
    }
    vkCmdDrawIndirectCount(commandBuffer,
                           buffers.at(drawParameterBufferRegionIndex->bufferIndex),
                           offset + bufferRegion(*drawParameterBufferRegionIndex).offset(),
                           buffers.at(countBufferRegionIndex->bufferIndex),
                           countBufferOffset + bufferRegion(*countBufferRegionIndex).offset(),
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
    CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
                                .bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferRegion(*index).offset());
    }
    if (indexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
        boundIndexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindIndexBuffer(commandBuffer, boundIndexBuffer, indexOffset + bufferRegion(*index).offset(), indexType);
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
    CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
                                .bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferRegion(*index).offset());
    }
    if (indexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
        boundIndexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindIndexBuffer(commandBuffer, boundIndexBuffer, indexOffset + bufferRegion(*index).offset(), indexType);
    }
    vkCmdDrawIndexedIndirect(commandBuffer,
                             buffers.at(bufferRegionIndex->bufferIndex),
                             offset + bufferRegion(*bufferRegionIndex).offset(),
                             drawCount,
                             stride);
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
    CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
                                .bind(*pipelineBindInfo));
    assert(commandBuffer.isRecording());
    if (vertexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
        boundVertexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &boundVertexBuffer, &bufferRegion(*index).offset());
    }
    if (indexBuffer != nullptr)
    {
        const buffer::BufferRegionIndex *index = static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
        boundIndexBuffer = buffers.at(index->bufferIndex);
        vkCmdBindIndexBuffer(commandBuffer, boundIndexBuffer, indexOffset + bufferRegion(*index).offset(), indexType);
    }
    vkCmdDrawIndexedIndirectCount(commandBuffer,
                                  buffers.at(drawParameterBufferRegionIndex->bufferIndex),
                                  offset + bufferRegion(*drawParameterBufferRegionIndex).offset(),
                                  buffers.at(countBufferRegionIndex->bufferIndex),
                                  countBufferOffset + bufferRegion(*countBufferRegionIndex).offset(),
                                  maxDrawCount,
                                  stride);
    return VK_SUCCESS;
}
