//
// Created by NBT22 on 2/12/25.
//

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/lunaBuffer.h>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "helpers/Handle.hpp"
#include "Luna.hpp"

static constexpr long double BLOCK_SIZE = 32 * 1024 * 1024;

namespace luna
{
BufferRegion::BufferRegion(Device &device,
                           const size_t size,
                           uint8_t *data,
                           const size_t offset,
                           Buffer *buffer,
                           LunaBuffer *outBuffer):
    size_(size),
    data_(data),
    offset_(offset)
{
    CHECK_RESULT_THROW(device.createBufferRegionIndex(buffer, this, outBuffer));
}

// TODO (0.3.0): This function needs to be properly reworked to consider buffer alignment
VkResult BufferRegion::findSpaceForBufferRegion(Device &device,
                                                const LunaBufferCreationInfo &creationInfo,
                                                Buffer *&outBuffer,
                                                size_t &outOffset,
                                                std::list<BufferRegion>::iterator &outIterator)
{
    constexpr VmaAllocationCreateInfo defaultAllocationCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };
    const VmaAllocationCreateInfo allocationCreateInfo = creationInfo.allocationCreateInfo == nullptr
                                                                 ? defaultAllocationCreateInfo
                                                                 : *creationInfo.allocationCreateInfo;

    if (creationInfo.alignment != 0)
    {
        const VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = creationInfo.flags,
            .size = static_cast<VkDeviceSize>(BLOCK_SIZE * std::max(std::ceil(creationInfo.size / BLOCK_SIZE), 1.0L)),
            .usage = creationInfo.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = creationInfo.queueFamilyIndexCount == 1 ? VK_SHARING_MODE_EXCLUSIVE
                                                                   : VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = creationInfo.queueFamilyIndexCount,
            .pQueueFamilyIndices = creationInfo.queueFamilyIndices,
        };
        CHECK_RESULT_RETURN(device.createBuffer(bufferCreateInfo,
                                                allocationCreateInfo,
                                                creationInfo.alignment,
                                                outBuffer));
        outOffset = 0;
        outIterator = outBuffer->regions_.end();
        return VK_SUCCESS;
    }

    // TODO (0.3.0): Uniform buffers have alignment requirements, and other buffer types may as well
    for (Buffer &buffer: device.buffers())
    {
        assert(std::ranges::is_sorted(buffer.regions_, [](const BufferRegion &a, const BufferRegion &b) -> bool {
            return a.offset() < b.offset();
        })); // Internal state check
        if (buffer.destroyed_ ||
            (buffer.usageFlags_ & creationInfo.usage) != creationInfo.usage ||
            (buffer.creationFlags_ & creationInfo.flags) != creationInfo.flags)
        {
            // This buffer is either destroyed or was created using flags incompatible with the new region
            continue;
        }

        /// TODO (0.3.0): Properly ensure allocation compatablility
        if (buffer.allocationCreateInfo_.flags != allocationCreateInfo.flags)
        {
            continue;
        }

        if (buffer.regions_.empty())
        {
            assert(buffer.usedBytes_ == 0 && buffer.unusedBytes_ == 0); // Internal state check
            if (buffer.freeBytes_ < creationInfo.size)
            {
                continue;
            }
            outBuffer = &buffer;
            outOffset = 0;
            outIterator = buffer.regions_.end();
            return VK_SUCCESS;
        }
        if (creationInfo.size <= buffer.unusedBytes_) // Buffer has enough dead space to fit the new region
        {
            if (creationInfo.size <= buffer.regions_.front().offset_) // New region can fit before the first region
            {
                outBuffer = &buffer;
                outOffset = 0;
                outIterator = buffer.regions_.begin();
                return VK_SUCCESS;
            }
            assert(buffer.regions_.size() != 1); // Internal state check
            const auto hasLargeEnoughGap = [&creationInfo](const BufferRegion &a, const BufferRegion &b) -> bool {
                return creationInfo.size <= b.offset() && a.offset() + a.size() < b.offset() - creationInfo.size;
            };
            const std::list<BufferRegion>::iterator regionIterator = std::ranges::adjacent_find(buffer.regions_,
                                                                                                hasLargeEnoughGap);
            if (regionIterator != buffer.regions_.end()) // We can fit the new region after regionIterator
            {
                outBuffer = &buffer;
                outOffset = regionIterator->offset() + regionIterator->size();
                outIterator = regionIterator;
                ++outIterator;
                assert(regionIterator != buffer.regions_.end());
                return VK_SUCCESS;
            }
            // No gap large enough to fit the new region was found, so continuing on to see if it can go at the end
        }
        if (creationInfo.size <= buffer.freeBytes_) // New region can fit at the end of the buffer
        {
            assert(buffer.usedBytes_ + buffer.unusedBytes_ ==
                   buffer.regions_.back().offset_ + buffer.regions_.back().size_); // Internal state check
            outBuffer = &buffer;
            outOffset = buffer.usedBytes_ + buffer.unusedBytes_;
            outIterator = buffer.regions_.end();
            return VK_SUCCESS;
        }
    }

    // No buffer was found that can fit the new region, so we will create a new buffer to hold the region
    const VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = creationInfo.flags,
        .size = static_cast<VkDeviceSize>(BLOCK_SIZE * std::max(std::ceil(creationInfo.size / BLOCK_SIZE), 1.0L)),
        .usage = creationInfo.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = creationInfo.queueFamilyIndexCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = creationInfo.queueFamilyIndexCount,
        .pQueueFamilyIndices = creationInfo.queueFamilyIndices,
    };
    CHECK_RESULT_RETURN(device.createBuffer(bufferCreateInfo, allocationCreateInfo, outBuffer));
    outOffset = 0;
    outIterator = outBuffer->regions_.end();
    return VK_SUCCESS;
}
VkResult BufferRegion::createBufferRegion(Device &device,
                                          const LunaBufferCreationInfo &creationInfo,
                                          LunaBuffer *outBuffer)
{
    Buffer *buffer{};
    size_t offset{};
    std::list<BufferRegion>::iterator iterator{};
    CHECK_RESULT_RETURN(findSpaceForBufferRegion(device, creationInfo, buffer, offset, iterator));

    if (creationInfo.size == 0)
    {
        CHECK_RESULT_RETURN(device.createBufferRegionIndex(buffer, nullptr, outBuffer));
        return VK_SUCCESS;
    }

    buffer->regions_.emplace(iterator,
                             device,
                             creationInfo.size,
                             buffer->data_ == nullptr ? nullptr : static_cast<uint8_t *>(buffer->data_) + offset,
                             offset,
                             buffer,
                             outBuffer);
    if (iterator == buffer->regions_.end())
    {
        buffer->freeBytes_ -= creationInfo.size;
    } else
    {
        buffer->unusedBytes_ -= creationInfo.size;
    }
    buffer->usedBytes_ += creationInfo.size;

    return VK_SUCCESS;
}

VkResult BufferRegionIndex::resize(Device &device, BufferRegionIndex *&bufferRegionIndex, VkDeviceSize newSize)
{
    if (bufferRegionIndex->size() == newSize)
    {
        return VK_SUCCESS;
    }

    if (bufferRegionIndex->bufferRegion_ == nullptr)
    {
        LunaBuffer lunaBuffer = LUNA_NULL_HANDLE;
        const LunaBufferCreationInfo newCreationInfo = {
            .size = newSize,
            .flags = bufferRegionIndex->buffer_->creationFlags_,
            .usage = bufferRegionIndex->buffer_->usageFlags_,
        };
        CHECK_RESULT_RETURN(BufferRegion::createBufferRegion(device, newCreationInfo, &lunaBuffer));

        bufferRegionIndex->destroy(static_cast<VkDevice>(device), device.allocator());
        bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(lunaBuffer);
    }

    const bool growing = bufferRegionIndex->size() < newSize;
    const size_t sizeChange = growing ? newSize - bufferRegionIndex->size() : bufferRegionIndex->size() - newSize;
    Buffer *buffer = bufferRegionIndex->buffer_;
    BufferRegion *bufferRegion = bufferRegionIndex->bufferRegion_;

    if (growing)
    {
        const bool regionIsLast = bufferRegion == &buffer->regions_.back();
        const bool regionCanBeResizedIntoFreeBytes = regionIsLast && newSize <= buffer->freeBytes_;
        bool regionCanBeResizedIntoUnusedBytes = false;
        if (!regionCanBeResizedIntoFreeBytes && !regionIsLast)
        {
            std::list<BufferRegion>::const_iterator bufferRegionIterator =
                    std::ranges::find_if(buffer->regions_, [&bufferRegion](const BufferRegion &region) -> bool {
                        return &region == bufferRegion;
                    });
            assert(bufferRegionIterator != buffer->regions_.cend()); // Internal state check
            ++bufferRegionIterator;
            assert(bufferRegionIterator != buffer->regions_.cend()); // Internal state check
            regionCanBeResizedIntoUnusedBytes = newSize <= bufferRegionIterator->offset_ - bufferRegion->offset_;
        }
        if (regionCanBeResizedIntoFreeBytes || regionCanBeResizedIntoUnusedBytes)
        {
            bufferRegion->size_ += sizeChange;
            buffer->usedBytes_ += sizeChange;
            if (regionCanBeResizedIntoFreeBytes)
            {
                buffer->freeBytes_ -= sizeChange;
            } else
            {
                buffer->unusedBytes_ -= sizeChange;
            }
        } else
        {
            LunaBuffer lunaBuffer = helpers::toHandle(bufferRegionIndex);
            LunaBufferCreationInfo newCreationInfo;
            bufferRegionIndex->creationInfo(newCreationInfo);
            newCreationInfo.size = newSize;

            CHECK_RESULT_RETURN(BufferRegion::createBufferRegion(device, newCreationInfo, &lunaBuffer));

            if (bufferRegion->data_ != nullptr)
            {
                uint8_t *newBufferRegionData = helpers::fromHandle<BufferRegionIndex>(lunaBuffer)->data();
                assert(newBufferRegionData != nullptr); // Internal state check
                std::copy_n(bufferRegion->data_, bufferRegion->size_, newBufferRegionData);
            }
            bufferRegionIndex->destroy(static_cast<VkDevice>(device), device.allocator());
            bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(lunaBuffer);
        }
    } else
    {
        bufferRegion->size_ -= sizeChange;
        buffer->usedBytes_ -= sizeChange;
        if (bufferRegion == &buffer->regions_.back())
        {
            buffer->freeBytes_ += sizeChange;
        } else
        {
            buffer->unusedBytes_ += sizeChange;
        }
    }

    return VK_SUCCESS;
}

void BufferRegionIndex::destroy(const VkDevice device, const VmaAllocator &allocator)
{
    for (const VkBufferView view: views_)
    {
        vkDestroyBufferView(device, view, nullptr);
    }

    if (bufferRegion_ != nullptr)
    {
        assert(buffer_);
        if (&buffer_->regions_.back() == bufferRegion_)
        {
            buffer_->freeBytes_ += bufferRegion_->size_;
        } else
        {
            buffer_->unusedBytes_ += bufferRegion_->size_;
        }
        buffer_->usedBytes_ -= bufferRegion_->size_;
        buffer_->regions_.remove_if([this](const BufferRegion &region) -> bool {
            return region.offset_ == bufferRegion_->offset_;
        });
        bufferRegion_ = nullptr;
    }

    if (buffer_ != nullptr && buffer_->regions_.empty())
    {
        buffer_->destroy(device, allocator);
        buffer_ = nullptr;
    }
}

VkResult BufferRegionIndex::flushMemory(const VmaAllocator &allocator) const
{
    CHECK_RESULT_RETURN(vmaFlushAllocation(allocator, buffer_->allocation_, offset(), size()));
    return VK_SUCCESS;
}

VkResult BufferRegionIndex::copyToBuffer(Device &device,
                                         CommandBuffer &commandBuffer,
                                         const uint8_t *data,
                                         const size_t bytes,
                                         const size_t offset,
                                         const VkPipelineStageFlags stageFlags) const
{
    assert(bytes <= size() - offset);

    uint8_t *mappedData = BufferRegionIndex::data();
    if (mappedData != nullptr)
    {
        std::copy_n(data, bytes, mappedData + offset);
        CHECK_RESULT_RETURN(flushMemory(device.allocator()));
        // TODO (0.3.0): Memory dependency for the provided stageFlags
        (void)stageFlags;
    } else
    {
        assert(this != device.stagingBuffer);
        CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(static_cast<VkDevice>(device)));
        CHECK_RESULT_RETURN(resize(device, device.stagingBuffer, bytes));
        assert(device.stagingBuffer->data() != nullptr);
        // ReSharper disable once CppDFANullDereference
        std::copy_n(data, bytes, device.stagingBuffer->data() + offset);
        const VkBufferCopy copyRegion = {
            .srcOffset = device.stagingBuffer->offset(),
            .dstOffset = BufferRegionIndex::offset() + offset,
            .size = bytes,
        };
        vkCmdCopyBuffer(commandBuffer, device.stagingBuffer->buffer(), buffer(), 1, &copyRegion);
    }
    return VK_SUCCESS;
}

VkResult BufferRegionIndex::createBufferView(const VkDevice device,
                                             const LunaBufferViewCreationInfo &creationInfo,
                                             LunaBufferView *lunaView)
{
    VkBufferView &view = views_.emplace_back();
    const VkBufferViewCreateInfo bufferViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
        .buffer = buffer(),
        .format = creationInfo.format,
        .offset = offset(),
        .range = size(),
    };
    CHECK_RESULT_RETURN(vkCreateBufferView(device, &bufferViewCreateInfo, nullptr, &view));

    if (lunaView != LUNA_NULL_HANDLE)
    {
        *lunaView = helpers::toHandle(view);
    }
    return VK_SUCCESS;
}


Buffer::Buffer(const VmaAllocator &allocator,
               const VkBufferCreateInfo &bufferCreateInfo,
               const VmaAllocationCreateInfo &allocationCreateInfo):
    creationFlags_(bufferCreateInfo.flags),
    usageFlags_(bufferCreateInfo.usage),
    allocationCreateInfo_(allocationCreateInfo),
    freeBytes_(bufferCreateInfo.size)
{
    VmaAllocationInfo allocationInfo;
    CHECK_RESULT_THROW(vmaCreateBuffer(allocator,
                                       &bufferCreateInfo,
                                       &allocationCreateInfo,
                                       &buffer_,
                                       &allocation_,
                                       &allocationInfo));
    data_ = allocationInfo.pMappedData;
    destroyed_ = false;
}
Buffer::Buffer(const VmaAllocator &allocator,
               const VkBufferCreateInfo &bufferCreateInfo,
               const VmaAllocationCreateInfo &allocationCreateInfo,
               const VkDeviceSize alignment):
    creationFlags_(bufferCreateInfo.flags),
    usageFlags_(bufferCreateInfo.usage),
    allocationCreateInfo_(allocationCreateInfo),
    freeBytes_(bufferCreateInfo.size)
{
    VmaAllocationInfo allocationInfo;
    CHECK_RESULT_THROW(vmaCreateBufferWithAlignment(allocator,
                                                    &bufferCreateInfo,
                                                    &allocationCreateInfo,
                                                    alignment,
                                                    &buffer_,
                                                    &allocation_,
                                                    &allocationInfo));
    data_ = allocationInfo.pMappedData;
    destroyed_ = false;
}

void Buffer::destroy(const VkDevice device, const VmaAllocator &allocator)
{
    assert(regions_.empty()); // Internal state check

    destroyed_ = true;
    vkDeviceWaitIdle(device); // TODO: This is a terrible solution
    vmaDestroyBuffer(allocator, buffer_, allocation_);
}
} // namespace luna

VkResult lunaCreateBuffer(const LunaDevice device, const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo && creationInfo->queueFamilyIndexCount != 0);
    CHECK_RESULT_RETURN(luna::BufferRegion::createBufferRegion(*luna::helpers::fromHandle<luna::Device>(device),
                                                               *creationInfo,
                                                               buffer));
    return VK_SUCCESS;
}

void lunaDestroyBuffer(const LunaDevice device, const LunaBuffer buffer)
{
    assert(device != LUNA_NULL_HANDLE);
    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    bufferRegionIndex->destroy(static_cast<VkDevice>(deviceObject), deviceObject.allocator());
}

VkResult lunaGrowBuffer(const LunaDevice device, LunaBuffer *buffer, const VkDeviceSize size)
{
    assert(buffer && *buffer != LUNA_NULL_HANDLE);
    if (luna::helpers::fromHandle<luna::BufferRegionIndex>(*buffer)->size() < size)
    {
        CHECK_RESULT_RETURN(lunaResizeBuffer(device, buffer, size));
    }
    return VK_SUCCESS;
}

VkResult lunaResizeBuffer(const LunaDevice device, LunaBuffer *buffer, const VkDeviceSize newSize)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(buffer && *buffer != LUNA_NULL_HANDLE);
    luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(*buffer);
    CHECK_RESULT_RETURN(luna::BufferRegionIndex::resize(*luna::helpers::fromHandle<luna::Device>(device),
                                                        bufferRegionIndex,
                                                        newSize));
    *buffer = luna::helpers::toHandle(bufferRegionIndex);
    return VK_SUCCESS;
}

VkResult lunaFillBuffer(const LunaDevice device,
                        const LunaCommandBuffer commandBuffer,
                        const LunaBuffer buffer,
                        const uint32_t data,
                        const VkQueue submissionQueue,
                        const VkPipelineStageFlags stageFlags)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(buffer != LUNA_NULL_HANDLE);

    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(static_cast<VkDevice>(deviceObject)));

    const luna::BufferRegionIndex &bufferRegionIndex = *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    vkCmdFillBuffer(commandBufferObject,
                    bufferRegionIndex.buffer(),
                    bufferRegionIndex.offset(),
                    bufferRegionIndex.size(),
                    data);
    CHECK_RESULT_RETURN(commandBufferObject.endAndSubmit(static_cast<VkDevice>(deviceObject),
                                                         submissionQueue,
                                                         stageFlags));
    return VK_SUCCESS;
}

VkResult lunaCreateBufferView(const LunaDevice device,
                              const LunaBufferViewCreationInfo *creationInfo,
                              LunaBufferView *bufferView)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo && creationInfo->buffer != LUNA_NULL_HANDLE);

    return luna::helpers::fromHandle<luna::BufferRegionIndex>(creationInfo->buffer)
            ->createBufferView(static_cast<VkDevice>(*luna::helpers::fromHandle<luna::Device>(device)),
                               *creationInfo,
                               bufferView);
}

VkResult lunaWriteDataToBuffer(const LunaDevice device,
                               const LunaCommandBuffer commandBuffer,
                               const LunaBuffer buffer,
                               const LunaBufferWriteInfo *writeInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(writeInfo != nullptr);
    if (writeInfo->bytes != 0)
    {
        assert(buffer != LUNA_NULL_HANDLE);
        assert(writeInfo->data != nullptr);

        const luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
        CHECK_RESULT_RETURN(bufferRegionIndex->copyToBuffer(*luna::helpers::fromHandle<luna::Device>(device),
                                                            *luna::helpers::fromHandle<
                                                                    luna::CommandBuffer>(commandBuffer),
                                                            static_cast<const uint8_t *>(writeInfo->data),
                                                            writeInfo->bytes,
                                                            writeInfo->offset,
                                                            writeInfo->stageFlags));
    }
    return VK_SUCCESS;
}

void *lunaGetBufferDataPointer(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer)->data();
}

VkDeviceSize lunaGetBufferSize(const LunaBuffer buffer)
{
    if (buffer == LUNA_NULL_HANDLE)
    {
        return 0;
    }
    // TODO: luna::BufferRegionIndex::size() returns a size_t but we are assuming it's the same size as VkDeviceSize
    return luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer)->size();
}

VkBufferCreateFlags lunaGetBufferCreationFlags(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer)->creationFlags();
}

VkBufferUsageFlags lunaGetBufferUsageFlags(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer)->usageFlags();
}

void lunaGetBufferAllocationCreateInfo(const LunaBuffer buffer, VmaAllocationCreateInfo *allocationCreateInfo)
{
    assert(buffer != LUNA_NULL_HANDLE);
    assert(allocationCreateInfo);
    luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer)->allocationCreateInfo(*allocationCreateInfo);
}

void lunaGetBufferCreationInfo(const LunaBuffer buffer,
                               LunaBufferCreationInfo *creationInfo,
                               VmaAllocationCreateInfo *allocationCreateInfo)
{
    assert(buffer != LUNA_NULL_HANDLE);
    assert(creationInfo);
    const luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    bufferRegionIndex->creationInfo(*creationInfo);
    if (allocationCreateInfo != nullptr)
    {
        bufferRegionIndex->allocationCreateInfo(*allocationCreateInfo);
    }
}

// Note: This is not guaranteed to work with a sparse buffer
VkDeviceAddress lunaGetBufferDeviceAddress(const LunaDevice device, const LunaBuffer buffer)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(buffer != LUNA_NULL_HANDLE);
    const luna::BufferRegionIndex &bufferRegionIndex = *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    const VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = bufferRegionIndex.buffer(),
    };
    return vkGetBufferDeviceAddress(static_cast<VkDevice>(*luna::helpers::fromHandle<luna::Device>(device)),
                                    &bufferDeviceAddressInfo) +
           bufferRegionIndex.offset();
}

VkBuffer lunaGetVkBuffer(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer)->buffer();
}

VkDeviceSize lunaGetBufferOffset(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer)->offset();
}

void lunaBindVertexBuffers(const LunaCommandBuffer commandBuffer,
                           const LunaBuffer *buffers,
                           const uint32_t firstBinding,
                           const uint32_t bindingCount)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(buffers);
    std::vector<VkBuffer> buffersVector;
    buffersVector.reserve(bindingCount);
    std::vector<VkDeviceSize> offsetsVector;
    offsetsVector.reserve(bindingCount);
    for (uint32_t i = 0; i < bindingCount; i++)
    {
        const luna::BufferRegionIndex *bufferRegionIndex =
                luna::helpers::fromHandle<luna::BufferRegionIndex>(buffers[i]);
        assert(bufferRegionIndex);
        buffersVector.emplace_back(bufferRegionIndex->buffer());
        offsetsVector.emplace_back(bufferRegionIndex->offset());
    }
    vkCmdBindVertexBuffers(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                           firstBinding,
                           bindingCount,
                           buffersVector.data(),
                           offsetsVector.data());
}

void lunaBindIndexBuffer(const LunaCommandBuffer commandBuffer, const LunaBuffer buffer, const VkIndexType indexType)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(buffer != LUNA_NULL_HANDLE);
    const luna::BufferRegionIndex *index = luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    vkCmdBindIndexBuffer(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                         index->buffer(),
                         index->offset(),
                         indexType);
}

VkResult lunaDrawBuffer(const LunaDevice device,
                        const LunaCommandBuffer commandBuffer,
                        const LunaBuffer vertexBuffer,
                        const LunaDrawInfo *drawInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    lunaBindVertexBuffers(commandBuffer, &vertexBuffer, 0, vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1);
    vkCmdDraw(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
              drawInfo->vertexCount,
              drawInfo->instanceCount,
              drawInfo->firstVertex,
              drawInfo->firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndirect(const LunaDevice device,
                                const LunaCommandBuffer commandBuffer,
                                const LunaBuffer vertexBuffer,
                                const LunaDrawIndirectInfo *drawInfo)
{
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    lunaBindVertexBuffers(commandBuffer, &vertexBuffer, 0, vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1);
    const luna::BufferRegionIndex *bufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    vkCmdDrawIndirect(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                      bufferRegionIndex->buffer(),
                      bufferRegionIndex->offset(),
                      drawInfo->drawCount,
                      drawInfo->stride == 0 ? sizeof(VkDrawIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndirectCount(const LunaDevice device,
                                     const LunaCommandBuffer commandBuffer,
                                     const LunaBuffer vertexBuffer,
                                     const LunaDrawIndirectCountInfo *drawInfo)
{
    assert(drawInfo &&
           drawInfo->pipeline != LUNA_NULL_HANDLE &&
           drawInfo->buffer != LUNA_NULL_HANDLE &&
           drawInfo->countBuffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    lunaBindVertexBuffers(commandBuffer, &vertexBuffer, 0, vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1);
    const luna::BufferRegionIndex *drawParameterBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    const luna::BufferRegionIndex *countBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->countBuffer);
    vkCmdDrawIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                           drawParameterBufferRegionIndex->buffer(),
                           drawParameterBufferRegionIndex->offset(),
                           countBufferRegionIndex->buffer(),
                           countBufferRegionIndex->offset(),
                           drawInfo->maxDrawCount,
                           drawInfo->stride == 0 ? sizeof(VkDrawIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexed(const LunaDevice device,
                               const LunaCommandBuffer commandBuffer,
                               const LunaBuffer vertexBuffer,
                               const LunaBuffer indexBuffer,
                               const VkIndexType indexType,
                               const LunaDrawIndexedInfo *drawInfo)
{
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    lunaBindVertexBuffers(commandBuffer, &vertexBuffer, 0, vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1);
    lunaBindIndexBuffer(commandBuffer, indexBuffer, indexType);
    vkCmdDrawIndexed(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                     drawInfo->indexCount,
                     drawInfo->instanceCount,
                     drawInfo->firstIndex,
                     drawInfo->vertexOffset,
                     drawInfo->firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirect(const LunaDevice device,
                                       const LunaCommandBuffer commandBuffer,
                                       const LunaBuffer vertexBuffer,
                                       const LunaBuffer indexBuffer,
                                       const VkIndexType indexType,
                                       const LunaDrawIndexedIndirectInfo *drawInfo)
{
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    const luna::BufferRegionIndex *bufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    lunaBindVertexBuffers(commandBuffer, &vertexBuffer, 0, vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1);
    lunaBindIndexBuffer(commandBuffer, indexBuffer, indexType);
    vkCmdDrawIndexedIndirect(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                             bufferRegionIndex->buffer(),
                             bufferRegionIndex->offset(),
                             drawInfo->drawCount,
                             drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirectCount(const LunaDevice device,
                                            const LunaCommandBuffer commandBuffer,
                                            const LunaBuffer vertexBuffer,
                                            const LunaBuffer indexBuffer,
                                            const VkIndexType indexType,
                                            const LunaDrawIndexedIndirectCountInfo *drawInfo)
{
    assert(drawInfo &&
           drawInfo->pipeline != LUNA_NULL_HANDLE &&
           drawInfo->buffer != LUNA_NULL_HANDLE &&
           drawInfo->countBuffer != LUNA_NULL_HANDLE);
    const luna::BufferRegionIndex *drawParameterBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    const luna::BufferRegionIndex *countBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->countBuffer);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    lunaBindVertexBuffers(commandBuffer, &vertexBuffer, 0, vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1);
    lunaBindIndexBuffer(commandBuffer, indexBuffer, indexType);
    vkCmdDrawIndexedIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                                  drawParameterBufferRegionIndex->buffer(),
                                  drawParameterBufferRegionIndex->offset(),
                                  countBufferRegionIndex->buffer(),
                                  countBufferRegionIndex->offset(),
                                  drawInfo->maxDrawCount,
                                  drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}
