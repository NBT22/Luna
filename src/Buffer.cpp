//
// Created by NBT22 on 2/12/25.
//

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <list>
#include <luna/lunaBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "helpers/Handle.hpp"
#include "Luna.hpp"

static constexpr long double BLOCK_SIZE = 32 * 1024 * 1024;

namespace luna
{
BufferRegion::BufferRegion(Device &device,
                           const VkDeviceSize size,
                           uint8_t *data,
                           const VkDeviceSize offset,
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
                                                VkDeviceSize &outOffset,
                                                std::list<BufferRegion>::iterator &outIterator)
{
    // TODO (0.3.0): Allow unmapped memory once possible
    constexpr VmaAllocationCreateInfo defaultAllocationCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
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
    VkDeviceSize offset{};
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

VkResult BufferRegionIndex::resize(Device &device,
                                   [[maybe_unused]] CommandBuffer &commandBuffer,
                                   BufferRegionIndex *&bufferRegionIndex,
                                   const VkDeviceSize newSize)
{
    if (bufferRegionIndex->size() == newSize)
    {
        return VK_SUCCESS;
    }

    if (bufferRegionIndex->bufferRegion_ == nullptr)
    {
        LunaBuffer lunaBuffer = LUNA_NULL_HANDLE;
        LunaBufferCreationInfo newCreationInfo{};
        bufferRegionIndex->creationInfo(newCreationInfo);
        newCreationInfo.size = newSize;
        CHECK_RESULT_RETURN(BufferRegion::createBufferRegion(device, newCreationInfo, &lunaBuffer));

        device.destroyBufferRegionIndex(bufferRegionIndex);
        bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(lunaBuffer);
    }

    const bool growing = bufferRegionIndex->size() < newSize;
    const VkDeviceSize sizeChange = growing ? newSize - bufferRegionIndex->size() : bufferRegionIndex->size() - newSize;
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
            LunaBuffer lunaBuffer = LUNA_NULL_HANDLE;
            LunaBufferCreationInfo newCreationInfo{};
            bufferRegionIndex->creationInfo(newCreationInfo);
            newCreationInfo.size = newSize;

            CHECK_RESULT_RETURN(BufferRegion::createBufferRegion(device, newCreationInfo, &lunaBuffer));
            assert(lunaBuffer != LUNA_NULL_HANDLE);
            // BufferRegionIndex &newBufferRegionIndex = *helpers::fromHandle<BufferRegionIndex>(lunaBuffer);

            // const VkBufferCopy copyRegion = {
            //     .srcOffset = bufferRegionIndex->offset(),
            //     .dstOffset = newBufferRegionIndex.offset(),
            //     .size = bufferRegionIndex->size(),
            // };
            // vkCmdCopyBuffer(commandBuffer, bufferRegionIndex->buffer(), newBufferRegionIndex.buffer(), 1, &copyRegion);

            // TODO (0.3.0): Once buffers are properly ref counted switch this to the commented out vkCmdCopyBuffer
            if (bufferRegion->data_ != nullptr)
            {
                uint8_t *newBufferRegionData = helpers::fromHandle<BufferRegionIndex>(lunaBuffer)->data();
                assert(newBufferRegionData != nullptr); // Internal state check
                std::copy_n(bufferRegion->data_, bufferRegion->size_, newBufferRegionData);
            }
            device.destroyBufferRegionIndex(bufferRegionIndex);
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

void BufferRegionIndex::destroy(Device &device)
{
    if (bufferRegion_ == nullptr)
    {
        assert(views_.empty());
        buffer_ = nullptr;
        return;
    }

    for (const VkBufferView view: views_)
    {
        vkDestroyBufferView(static_cast<VkDevice>(device), view, nullptr);
    }

    assert(buffer_);
    if (&buffer_->regions_.back() == bufferRegion_)
    {
        buffer_->freeBytes_ += bufferRegion_->size_;
    } else
    {
        buffer_->unusedBytes_ += bufferRegion_->size_;
    }
    buffer_->usedBytes_ -= bufferRegion_->size_;
    buffer_->regions_.remove_if([this](const BufferRegion &region) -> bool { return &region == bufferRegion_; });
    bufferRegion_ = nullptr;

    if (buffer_ != nullptr && buffer_->regions_.empty())
    {
        device.destroyBuffer(buffer_);
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
                                         const VkDeviceSize bytes,
                                         const VkDeviceSize offset,
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
        BufferRegionIndex *&stagingBuffer = device.stagingBuffer();
        assert(this != stagingBuffer && stagingBuffer != nullptr);
        CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(static_cast<VkDevice>(device)));
        CHECK_RESULT_RETURN(resize(device, commandBuffer, stagingBuffer, bytes));
        assert(stagingBuffer->data() != nullptr);
        std::copy_n(data, bytes, stagingBuffer->data() + offset);
        const VkBufferCopy copyRegion = {
            .srcOffset = stagingBuffer->offset(),
            .dstOffset = BufferRegionIndex::offset() + offset,
            .size = bytes,
        };
        vkCmdCopyBuffer(commandBuffer, stagingBuffer->buffer(), buffer(), 1, &copyRegion);
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
    freeBytes_(bufferCreateInfo.size),
    queueFamilyIndices_(bufferCreateInfo.pQueueFamilyIndices,
                        bufferCreateInfo.pQueueFamilyIndices + bufferCreateInfo.queueFamilyIndexCount)
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
    freeBytes_(bufferCreateInfo.size),
    queueFamilyIndices_(bufferCreateInfo.pQueueFamilyIndices,
                        bufferCreateInfo.pQueueFamilyIndices + bufferCreateInfo.queueFamilyIndexCount)
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
    assert(!destroyed_); // Internal state check
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
    luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    deviceObject.destroyBufferRegionIndex(bufferRegionIndex);
}

VkResult lunaGrowBuffer(const LunaDevice device,
                        const LunaCommandBuffer commandBuffer,
                        LunaBuffer *buffer,
                        const VkDeviceSize size)
{
    assert(buffer && *buffer != LUNA_NULL_HANDLE);
    if (luna::helpers::fromHandle<luna::BufferRegionIndex>(*buffer)->size() < size)
    {
        CHECK_RESULT_RETURN(lunaResizeBuffer(device, commandBuffer, buffer, size));
    }
    return VK_SUCCESS;
}

VkResult lunaResizeBuffer(const LunaDevice device,
                          const LunaCommandBuffer commandBuffer,
                          LunaBuffer *buffer,
                          const VkDeviceSize newSize)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(buffer && *buffer != LUNA_NULL_HANDLE);
    luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(*buffer);
    CHECK_RESULT_RETURN(luna::BufferRegionIndex::resize(*luna::helpers::fromHandle<luna::Device>(device),
                                                        *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                                                        bufferRegionIndex,
                                                        newSize));
    *buffer = luna::helpers::toHandle(bufferRegionIndex);
    return VK_SUCCESS;
}

VkResult lunaFillBuffer(const LunaDevice device,
                        const LunaCommandBuffer commandBuffer,
                        const LunaBuffer buffer,
                        const uint32_t data,
                        const LunaCommandBufferSubmitInfo *submitInfo)
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
    if (submitInfo != nullptr)
    {
        CHECK_RESULT_RETURN(commandBufferObject.endAndSubmit(static_cast<VkDevice>(deviceObject), *submitInfo));
    }
    return VK_SUCCESS;
}

VkResult lunaWriteUintToBuffer(const LunaDevice device,
                               const LunaCommandBuffer commandBuffer,
                               const LunaBuffer buffer,
                               const VkDeviceSize offset,
                               const uint32_t value,
                               const LunaCommandBufferSubmitInfo *submitInfo)
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
                    bufferRegionIndex.offset() + offset,
                    sizeof(uint32_t),
                    value);
    if (submitInfo != nullptr)
    {
        CHECK_RESULT_RETURN(commandBufferObject.endAndSubmit(static_cast<VkDevice>(deviceObject), *submitInfo));
    }
    return VK_SUCCESS;
}

VkResult lunaCreateBufferView(const LunaDevice device,
                              const LunaBufferViewCreationInfo *creationInfo,
                              LunaBufferView *bufferView)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo && creationInfo->buffer != LUNA_NULL_HANDLE);

    return luna::helpers::fromHandle<luna::BufferRegionIndex>(creationInfo->buffer)
            ->createBufferView(lunaGetVkDevice(device), *creationInfo, bufferView);
}

VkBufferView lunaGetVkBufferView(const LunaBufferView bufferView)
{
    assert(bufferView != LUNA_NULL_HANDLE);
    return *luna::helpers::fromHandle<VkBufferView>(bufferView);
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
    if (bufferRegionIndex.size() == 0)
    {
        return 0;
    }
    const VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = bufferRegionIndex.buffer(),
    };
    return vkGetBufferDeviceAddress(lunaGetVkDevice(device), &bufferDeviceAddressInfo) + bufferRegionIndex.offset();
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

VkResult lunaBindVertexBuffers(const LunaDevice device,
                               const LunaCommandBuffer commandBuffer,
                               const LunaBuffer *buffers,
                               const uint32_t firstBinding,
                               const uint32_t bindingCount)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(buffers);

    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(lunaGetVkDevice(device)));

    std::vector<VkBuffer> buffersVector;
    buffersVector.reserve(bindingCount);
    std::vector<VkDeviceSize> offsetsVector;
    offsetsVector.reserve(bindingCount);
    for (uint32_t i = 0; i < bindingCount; i++)
    {
        assert(buffers[i] != LUNA_NULL_HANDLE);
        const luna::BufferRegionIndex &bufferRegionIndex =
                *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffers[i]);
        buffersVector.emplace_back(bufferRegionIndex.buffer());
        offsetsVector.emplace_back(bufferRegionIndex.offset());
    }
    vkCmdBindVertexBuffers(commandBufferObject, firstBinding, bindingCount, buffersVector.data(), offsetsVector.data());
    return VK_SUCCESS;
}

VkResult lunaBindIndexBuffer(const LunaDevice device,
                             const LunaCommandBuffer commandBuffer,
                             const LunaBuffer buffer,
                             const VkIndexType indexType)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(buffer != LUNA_NULL_HANDLE);

    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(lunaGetVkDevice(device)));

    const luna::BufferRegionIndex &index = *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    vkCmdBindIndexBuffer(commandBufferObject, index.buffer(), index.offset(), indexType);
    return VK_SUCCESS;
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
    CHECK_RESULT_RETURN(lunaBindVertexBuffers(device,
                                              commandBuffer,
                                              &vertexBuffer,
                                              0,
                                              vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1));
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
    CHECK_RESULT_RETURN(lunaBindVertexBuffers(device,
                                              commandBuffer,
                                              &vertexBuffer,
                                              0,
                                              vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1));
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
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    CHECK_RESULT_RETURN(lunaBindVertexBuffers(device,
                                              commandBuffer,
                                              &vertexBuffer,
                                              0,
                                              vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1));
    const luna::BufferRegionIndex &drawParameterBufferRegionIndex =
            *luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    vkCmdDrawIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                           drawParameterBufferRegionIndex.buffer(),
                           drawParameterBufferRegionIndex.offset() + sizeof(uint32_t),
                           drawParameterBufferRegionIndex.buffer(),
                           drawParameterBufferRegionIndex.offset(),
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
    CHECK_RESULT_RETURN(lunaBindVertexBuffers(device,
                                              commandBuffer,
                                              &vertexBuffer,
                                              0,
                                              vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1));
    CHECK_RESULT_RETURN(lunaBindIndexBuffer(device, commandBuffer, indexBuffer, indexType));
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
    CHECK_RESULT_RETURN(lunaBindVertexBuffers(device,
                                              commandBuffer,
                                              &vertexBuffer,
                                              0,
                                              vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1));
    CHECK_RESULT_RETURN(lunaBindIndexBuffer(device, commandBuffer, indexBuffer, indexType));
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
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    const luna::BufferRegionIndex &drawParameterBufferRegionIndex =
            *luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    CHECK_RESULT_RETURN(lunaBindVertexBuffers(device,
                                              commandBuffer,
                                              &vertexBuffer,
                                              0,
                                              vertexBuffer == LUNA_NULL_HANDLE ? 0 : 1));
    CHECK_RESULT_RETURN(lunaBindIndexBuffer(device, commandBuffer, indexBuffer, indexType));
    vkCmdDrawIndexedIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                                  drawParameterBufferRegionIndex.buffer(),
                                  drawParameterBufferRegionIndex.offset() + sizeof(uint32_t),
                                  drawParameterBufferRegionIndex.buffer(),
                                  drawParameterBufferRegionIndex.offset(),
                                  drawInfo->maxDrawCount,
                                  drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}
