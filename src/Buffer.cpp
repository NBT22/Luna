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
#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <map>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "helpers/Handle.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

static constexpr long double BLOCK_SIZE = 32 * 1024 * 1024;

namespace luna
{
BufferRegion::BufferRegion(const size_t size,
                           uint8_t *data,
                           const size_t offset,
                           std::map<const char *, SubRegion> &&subRegions,
                           Buffer *buffer,
                           LunaBuffer *outBuffer):
    size_(size),
    data_(data),
    offset_(offset),
    subRegions_(std::move(subRegions))
{
    bufferRegionIndices.emplace_back(buffer, this);
    if (outBuffer != nullptr)
    {
        *outBuffer = helpers::toHandle(&bufferRegionIndices.back());
    }
}

// TODO (0.3.0): This function needs to be properly reworked to consider buffer alignment
VkResult BufferRegion::findSpaceForBufferRegion(const LunaBufferCreationInfo &creationInfo,
                                                Buffer *&outBuffer,
                                                size_t &outOffset,
                                                std::list<BufferRegion>::iterator &outIterator)
{
    if (creationInfo.alignment != 0)
    {
        const VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = creationInfo.flags,
            .size = static_cast<VkDeviceSize>(BLOCK_SIZE * std::max(std::ceil(creationInfo.size / BLOCK_SIZE), 1.0L)),
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
                                                            : allocationCreateInfo,
                                                    creationInfo.alignment));
        outBuffer = &buffers.back();
        outOffset = 0;
        outIterator = outBuffer->regions_.end();
        return VK_SUCCESS;
    }

    // TODO (0.3.0): Uniform buffers have alignment requirements, and other buffer types may as well
    for (Buffer &buffer: buffers)
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
        if (buffer.allocationCreateInfo_.flags != creationInfo.allocationCreateInfo->flags)
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
                                                creationInfo.allocationCreateInfo ? *creationInfo.allocationCreateInfo
                                                                                  : allocationCreateInfo));
    outBuffer = &buffers.back();
    outOffset = 0;
    outIterator = outBuffer->regions_.end();
    return VK_SUCCESS;
}
VkResult BufferRegion::createBufferRegion(const LunaBufferCreationInfo &creationInfo, LunaBuffer *outBuffer)
{
    std::map<const char *, SubRegion> subRegions;
    VkDeviceSize combinedRegionSizes = 0;
    for (uint32_t i = 0; i < creationInfo.regionCount; i++)
    {
        subRegions.emplace(std::piecewise_construct,
                           std::forward_as_tuple(creationInfo.regionNames[i]),
                           std::forward_as_tuple(creationInfo.regionSizes[i], combinedRegionSizes));
        combinedRegionSizes += creationInfo.regionSizes[i];
        assert(combinedRegionSizes <= creationInfo.size);
    }

    Buffer *buffer{};
    size_t offset{};
    std::list<BufferRegion>::iterator iterator{};
    CHECK_RESULT_RETURN(findSpaceForBufferRegion(creationInfo, buffer, offset, iterator));

    if (creationInfo.size == 0)
    {
        bufferRegionIndices.emplace_back(buffer, nullptr);
        if (outBuffer != nullptr)
        {
            *outBuffer = helpers::toHandle(&bufferRegionIndices.back());
        }
        return VK_SUCCESS;
    }

    buffer->regions_.emplace(iterator,
                             creationInfo.size,
                             buffer->data_ == nullptr ? nullptr : static_cast<uint8_t *>(buffer->data_) + offset,
                             offset,
                             std::move(subRegions),
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


void BufferRegionIndex::destroy(BufferRegionIndex *const &bufferRegionIndex)
{
    const auto shouldDestroy = [&bufferRegionIndex](const BufferRegionIndex &regionIndex) -> bool {
        return &regionIndex == bufferRegionIndex;
    };
    [[maybe_unused]] const size_t removedRegions = bufferRegionIndices.remove_if(shouldDestroy);
    assert(removedRegions < 2);
}

void BufferRegionIndex::destroy(BufferRegionIndex *&bufferRegionIndex)
{
    const auto shouldDestroy = [&bufferRegionIndex](const BufferRegionIndex &regionIndex) -> bool {
        return &regionIndex == bufferRegionIndex;
    };
    const size_t removedRegions = bufferRegionIndices.remove_if(shouldDestroy);
    assert(removedRegions < 2);
    if (removedRegions != 0)
    {
        bufferRegionIndex = nullptr;
    }
}

BufferRegionIndex::~BufferRegionIndex()
{
    for (const VkBufferView view: views_)
    {
        vkDestroyBufferView(device, view, nullptr);
    }

    if (bufferRegion_ != nullptr)
    {
        assert(buffer_);
        if (subRegion_ != nullptr)
        {
            if (&buffer_->regions_.back() == bufferRegion_ &&
                (bufferRegion_->subRegions_.size() == 1 || subRegion_->offset != 0))
            {
                buffer_->freeBytes_ += subRegion_->size;
            } else
            {
                buffer_->unusedBytes_ += subRegion_->size;
            }
            buffer_->usedBytes_ -= subRegion_->size;
            bufferRegion_->size_ -= subRegion_->size;
            if (subRegion_->offset == 0)
            {
                bufferRegion_->offset_ += subRegion_->size;
                if (bufferRegion_->data_ != nullptr)
                {
                    bufferRegion_->data_ += subRegion_->size;
                }
            }

            if (bufferRegion_->subRegions_.size() == 1)
            {
                bufferRegion_->subRegions_.clear();
            } else
            {
                const decltype(bufferRegion_->subRegions_)::iterator iterator =
                        std::ranges::find_if(bufferRegion_->subRegions_,
                                             [this](decltype(bufferRegion_->subRegions_)::const_reference pair)
                                                     -> bool { return &pair.second == subRegion_; });
                assert(iterator != bufferRegion_->subRegions_.end());
                std::ranges::for_each(bufferRegion_->subRegions_ | std::views::values,
                                      [this](SubRegion &subRegion) -> void {
                                          if (subRegion_->offset < subRegion.offset)
                                          {
                                              subRegion.offset -= subRegion_->size;
                                          }
                                      });
                for (SubRegion &subRegion: bufferRegion_->subRegions_ | std::views::values)
                {
                    if (subRegion_->offset < subRegion.offset)
                    {
                        subRegion.offset -= subRegion_->size;
                    }
                }
                bufferRegion_->subRegions_.erase(iterator);
            }
        } else
        {
            if (&buffer_->regions_.back() == bufferRegion_)
            {
                buffer_->freeBytes_ += bufferRegion_->size_;
            } else
            {
                buffer_->unusedBytes_ += bufferRegion_->size_;
            }
            buffer_->usedBytes_ -= bufferRegion_->size_;
        }
        if (bufferRegion_->subRegions_.empty())
        {
            buffer_->regions_.remove_if([this](const BufferRegion &region) -> bool {
                return region.offset_ == bufferRegion_->offset_;
            });
        }
    }

    if (buffer_->regions_.empty())
    {
        // TODO: This system is scuffed at best and severely bug-prone at worst
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

VkResult BufferRegionIndex::flushMemory() const
{
    CHECK_RESULT_RETURN(vmaFlushAllocation(device.allocator(), buffer_->allocation_, offset(), size()));
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
        CHECK_RESULT_RETURN(flushMemory());
        // TODO (0.3.0): Memory dependency for the provided stageFlags
    } else
    {
        assert(this != stagingBuffer);
        // TODO: Should this use a dedicated transfer command buffer
        CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer(1);
        CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(true));
        CHECK_RESULT_RETURN(resize(stagingBuffer, bytes));
        assert(stagingBuffer->data() != nullptr);
        // ReSharper disable once CppDFANullDereference
        std::copy_n(data, bytes, stagingBuffer->data() + offset);
        const VkBufferCopy copyRegion = {
            .srcOffset = stagingBuffer->offset(),
            .dstOffset = BufferRegionIndex::offset() + offset,
            .size = bytes,
        };
        vkCmdCopyBuffer(commandBuffer, stagingBuffer->buffer(), buffer(), 1, &copyRegion);
        CHECK_RESULT_RETURN(commandBuffer.endAndSubmit(luna::device.familyQueues().graphics, stageFlags));
    }
    return VK_SUCCESS;
}

VkResult BufferRegionIndex::createBufferView(const LunaBufferViewCreationInfo &creationInfo, LunaBufferView *lunaView)
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


Buffer::Buffer(const VkBufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo):
    creationFlags_(bufferCreateInfo.flags),
    usageFlags_(bufferCreateInfo.usage),
    allocationCreateInfo_(allocationCreateInfo),
    freeBytes_(bufferCreateInfo.size)
{
    BufferRegionIndex::waitForCleanupThread();

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
Buffer::Buffer(const VkBufferCreateInfo &bufferCreateInfo,
               const VmaAllocationCreateInfo &allocationCreateInfo,
               const VkDeviceSize alignment):
    creationFlags_(bufferCreateInfo.flags),
    usageFlags_(bufferCreateInfo.usage),
    allocationCreateInfo_(allocationCreateInfo),
    freeBytes_(bufferCreateInfo.size)
{
    BufferRegionIndex::waitForCleanupThread();

    VmaAllocationInfo allocationInfo;
    CHECK_RESULT_THROW(vmaCreateBufferWithAlignment(device.allocator(),
                                                    &bufferCreateInfo,
                                                    &allocationCreateInfo,
                                                    alignment,
                                                    &buffer_,
                                                    &allocation_,
                                                    &allocationInfo));
    data_ = allocationInfo.pMappedData;
    destroyed_ = false;
}

Buffer::~Buffer()
{
    assert(regions_.empty()); // Internal state check

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

void lunaDestroyBuffer(const LunaBuffer buffer)
{
    luna::BufferRegionIndex::destroy(luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer));
}

VkResult lunaGrowBuffer(LunaBuffer *buffer, const VkDeviceSize size)
{
    assert(buffer && *buffer != LUNA_NULL_HANDLE);
    if (luna::helpers::fromHandle<luna::BufferRegionIndex>(*buffer)->size() < size)
    {
        return lunaResizeBuffer(buffer, size);
    }
    return VK_SUCCESS;
}

VkResult lunaResizeBuffer(LunaBuffer *buffer, const VkDeviceSize newSize)
{
    assert(buffer && *buffer != LUNA_NULL_HANDLE);
    luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(*buffer);
    CHECK_RESULT_RETURN(luna::BufferRegionIndex::resize(bufferRegionIndex, newSize));
    *buffer = luna::helpers::toHandle(bufferRegionIndex);
    return VK_SUCCESS;
}

VkResult lunaFillBuffer(const LunaBuffer buffer, const uint32_t data)
{
    assert(buffer != LUNA_NULL_HANDLE);

    // TODO (0.3.0): Command buffer needs to be picked such that it will actually be submitted. This applies to all
    //  command buffers and is an important consideration, since headless compute doesn't have the render loop that much
    //  of the library assumes will always be present.
    luna::CommandBuffer &commandBuffer = luna::device.commandPools().compute->commandBuffer(0);
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(true));

    const luna::BufferRegionIndex &bufferRegionIndex = *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    vkCmdFillBuffer(commandBuffer,
                    bufferRegionIndex.buffer(),
                    bufferRegionIndex.offset(),
                    bufferRegionIndex.size(),
                    data);
    // TODO (0.3.0): This is not ideal
    commandBuffer.endAndSubmit(luna::device.familyQueues().compute, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    return VK_SUCCESS;
}

VkResult lunaCreateBufferView(const LunaBufferViewCreationInfo *creationInfo, LunaBufferView *bufferView)
{
    assert(creationInfo);
    assert(creationInfo->buffer != LUNA_NULL_HANDLE);

    return luna::helpers::fromHandle<luna::BufferRegionIndex>(creationInfo->buffer)
            ->createBufferView(*creationInfo, bufferView);
}

VkResult lunaWriteDataToBuffer(const LunaBuffer buffer, const LunaBufferWriteInfo *writeInfo)
{
    assert(writeInfo != nullptr);
    if (writeInfo->bytes != 0)
    {
        assert(buffer != LUNA_NULL_HANDLE);
        assert(writeInfo->data != nullptr);

        const luna::BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
        CHECK_RESULT_RETURN(bufferRegionIndex->copyToBuffer(static_cast<const uint8_t *>(writeInfo->data),
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
VkDeviceAddress lunaGetBufferDeviceAddress(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    const luna::BufferRegionIndex &bufferRegionIndex = *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    const VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = bufferRegionIndex.buffer(),
    };
    return vkGetBufferDeviceAddress(luna::device, &bufferDeviceAddressInfo) + bufferRegionIndex.offset();
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

void lunaBindVertexBuffers(const LunaBuffer *buffers, const uint32_t firstBinding, const uint32_t bindingCount)
{
    if (bindingCount == 0)
    {
        return;
    }
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
    vkCmdBindVertexBuffers(luna::device.commandPools().graphics->commandBuffer(),
                           firstBinding,
                           bindingCount,
                           buffersVector.data(),
                           offsetsVector.data());
}

void lunaBindIndexBuffer(const LunaBuffer buffer, const VkIndexType indexType)
{
    assert(buffer != LUNA_NULL_HANDLE);
    const luna::BufferRegionIndex *index = luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);
    vkCmdBindIndexBuffer(luna::device.commandPools().graphics->commandBuffer(),
                         index->buffer(),
                         index->offset(),
                         indexType);
}

VkResult lunaDrawBuffer(const LunaBuffer vertexBuffer, const LunaDrawInfo *drawInfo)
{
    using namespace luna;
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE);
    const CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer();
    CHECK_RESULT_RETURN(helpers::fromHandle<GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(commandBuffer.isRecording()); // Internal state check
    if (vertexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(vertexBuffer);
        const size_t bufferOffset = index->offset();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
    }
    vkCmdDraw(commandBuffer,
              drawInfo->vertexCount,
              drawInfo->instanceCount,
              drawInfo->firstVertex,
              drawInfo->firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndirect(const LunaBuffer vertexBuffer, const LunaDrawIndirectInfo *drawInfo)
{
    using namespace luna;
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    const BufferRegionIndex *bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(drawInfo->buffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer();
    CHECK_RESULT_RETURN(helpers::fromHandle<GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(commandBuffer.isRecording()); // Internal state check
    if (vertexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(vertexBuffer);
        const size_t bufferOffset = index->offset();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
    }
    vkCmdDrawIndirect(commandBuffer,
                      bufferRegionIndex->buffer(),
                      bufferRegionIndex->offset(),
                      drawInfo->drawCount,
                      drawInfo->stride == 0 ? sizeof(VkDrawIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndirectCount(const LunaBuffer vertexBuffer, const LunaDrawIndirectCountInfo *drawInfo)
{
    using namespace luna;
    assert(drawInfo &&
           drawInfo->pipeline != LUNA_NULL_HANDLE &&
           drawInfo->buffer != LUNA_NULL_HANDLE &&
           drawInfo->countBuffer != LUNA_NULL_HANDLE);
    const BufferRegionIndex *drawParameterBufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(drawInfo->buffer);
    const BufferRegionIndex *countBufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(drawInfo->countBuffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer();
    CHECK_RESULT_RETURN(helpers::fromHandle<GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(commandBuffer.isRecording()); // Internal state check
    if (vertexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(vertexBuffer);
        const size_t bufferOffset = index->offset();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
    }
    vkCmdDrawIndirectCount(commandBuffer,
                           drawParameterBufferRegionIndex->buffer(),
                           drawParameterBufferRegionIndex->offset(),
                           countBufferRegionIndex->buffer(),
                           countBufferRegionIndex->offset(),
                           drawInfo->maxDrawCount,
                           drawInfo->stride == 0 ? sizeof(VkDrawIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexed(const LunaBuffer vertexBuffer,
                               const LunaBuffer indexBuffer,
                               const VkIndexType indexType,
                               const LunaDrawIndexedInfo *drawInfo)
{
    using namespace luna;
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE);
    const CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer();
    CHECK_RESULT_RETURN(helpers::fromHandle<GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(commandBuffer.isRecording()); // Internal state check
    if (vertexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(vertexBuffer);
        const size_t bufferOffset = index->offset();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
    }
    if (indexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(indexBuffer);
        vkCmdBindIndexBuffer(commandBuffer, index->buffer(), index->offset(), indexType);
    }
    vkCmdDrawIndexed(commandBuffer,
                     drawInfo->indexCount,
                     drawInfo->instanceCount,
                     drawInfo->firstIndex,
                     drawInfo->vertexOffset,
                     drawInfo->firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirect(const LunaBuffer vertexBuffer,
                                       const LunaBuffer indexBuffer,
                                       const VkIndexType indexType,
                                       const LunaDrawIndexedIndirectInfo *drawInfo)
{
    using namespace luna;
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    const BufferRegionIndex *bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(drawInfo->buffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer();
    CHECK_RESULT_RETURN(helpers::fromHandle<GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(commandBuffer.isRecording()); // Internal state check
    if (vertexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(vertexBuffer);
        const size_t bufferOffset = index->offset();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
    }
    if (indexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(indexBuffer);
        vkCmdBindIndexBuffer(commandBuffer, index->buffer(), index->offset(), indexType);
    }
    vkCmdDrawIndexedIndirect(commandBuffer,
                             bufferRegionIndex->buffer(),
                             bufferRegionIndex->offset(),
                             drawInfo->drawCount,
                             drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirectCount(const LunaBuffer vertexBuffer,
                                            const LunaBuffer indexBuffer,
                                            const VkIndexType indexType,
                                            const LunaDrawIndexedIndirectCountInfo *drawInfo)
{
    using namespace luna;
    assert(drawInfo &&
           drawInfo->pipeline != LUNA_NULL_HANDLE &&
           drawInfo->buffer != LUNA_NULL_HANDLE &&
           drawInfo->countBuffer != LUNA_NULL_HANDLE);
    const BufferRegionIndex *drawParameterBufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(drawInfo->buffer);
    const BufferRegionIndex *countBufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(drawInfo->countBuffer);
    const CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer();
    CHECK_RESULT_RETURN(helpers::fromHandle<GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(commandBuffer.isRecording()); // Internal state check
    if (vertexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(vertexBuffer);
        const size_t bufferOffset = index->offset();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &index->buffer(), &bufferOffset);
    }
    if (indexBuffer != LUNA_NULL_HANDLE)
    {
        const BufferRegionIndex *index = helpers::fromHandle<BufferRegionIndex>(indexBuffer);
        vkCmdBindIndexBuffer(commandBuffer, index->buffer(), index->offset(), indexType);
    }
    vkCmdDrawIndexedIndirectCount(commandBuffer,
                                  drawParameterBufferRegionIndex->buffer(),
                                  drawParameterBufferRegionIndex->offset(),
                                  countBufferRegionIndex->buffer(),
                                  countBufferRegionIndex->offset(),
                                  drawInfo->maxDrawCount,
                                  drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}
