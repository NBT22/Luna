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

namespace luna
{
Buffer::Buffer(const LunaMemoryType memoryType,
               const VkBufferUsageFlags additionalUsageFlags,
               const VmaAllocator &allocator,
               const VkDeviceSize size):
    allocator_(allocator),
    memoryType_(memoryType),
    additionalUsageFlags_(additionalUsageFlags)
{
    assert(size > 0 && size % (VkDeviceSize{32} * 1024 * 1024) == 0);
    const VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                 VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
                 additionalUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VmaAllocationInfo allocationInfo;
    CHECK_RESULT_THROW(vmaCreateBuffer(allocator,
                                       &bufferCreateInfo,
                                       &allocationCreateInfo(memoryType),
                                       &buffer_,
                                       &allocation_,
                                       &allocationInfo));
    const VmaVirtualBlockCreateInfo virtualBlockCreateInfo = {
        .size = size,
    };
    CHECK_RESULT_THROW(vmaCreateVirtualBlock(&virtualBlockCreateInfo, &virtualBlock_));
    data_ = allocationInfo.pMappedData;
}

VkResult BufferRegion::writeData(Device &device, CommandBuffer &commandBuffer, const LunaBufferWriteInfo &writeInfo)
{
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(static_cast<VkDevice>(device)));

    const bool smallWrite = writeInfo.size < 4096;

    const bool useSynchronizationType = writeInfo.sourceStageMask != VK_PIPELINE_STAGE_2_NONE &&
                                        writeInfo.sourceAccessMask == VK_ACCESS_2_NONE &&
                                        writeInfo.destinationStageMask == VK_PIPELINE_STAGE_2_NONE &&
                                        writeInfo.destinationAccessMask == VK_ACCESS_2_NONE;

    VkPipelineStageFlags2 stageMask = writeInfo.sourceStageMask;
    VkAccessFlags2 accessMask = writeInfo.sourceAccessMask;
    if (useSynchronizationType)
    {
        switch (writeInfo.synchronizationType)
        {
            case LUNA_BUFFER_SYNCHRONIZATION_TYPE_VERTEX_BUFFER:
                stageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
                accessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                break;
            case LUNA_BUFFER_SYNCHRONIZATION_TYPE_INDEX_BUFFER:
                stageMask = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
                accessMask = VK_ACCESS_2_INDEX_READ_BIT;
                break;
            case LUNA_BUFFER_SYNCHRONIZATION_TYPE_INDIRECT_BUFFER:
                stageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
                accessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                break;
            case LUNA_BUFFER_SYNCHRONIZATION_TYPE_FRAGMENT_SHADER_UNIFORM_BUFFER:
                stageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                accessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
                break;
            case LUNA_BUFFER_SYNCHRONIZATION_TYPE_UNIFORM_BUFFER:
                stageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
                            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                accessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
                break;
            case LUNA_BUFFER_SYNCHRONIZATION_TYPE_COMPUTE_SHADER_STORAGE_BUFFER:
                stageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                accessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                break;
            default:
                break;
        }
    }

    if (stageMask != VK_PIPELINE_STAGE_2_NONE && accessMask != VK_ACCESS_2_NONE)
    {
        const LunaBufferMemoryBarrier preWriteBarrier = {
            .sourceStageMask = stageMask,
            .sourceAccessMask = stageMask,
            .destinationStageMask = smallWrite ? VK_PIPELINE_STAGE_2_CLEAR_BIT : VK_PIPELINE_STAGE_2_COPY_BIT,
            .destinationAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .buffer = helpers::toHandle(this),
        };
        const LunaDependencyInfo preWriteDependencyInfo = {
            .bufferMemoryBarrierCount = 1,
            .bufferMemoryBarriers = &preWriteBarrier,
        };
        helpers::pipelineBarrier(commandBuffer, preWriteDependencyInfo);
    }

    if (smallWrite)
    {
        vkCmdUpdateBuffer(commandBuffer, buffer(), offset() + writeInfo.offset, writeInfo.size, writeInfo.data);
    } else
    {
        const BufferRegion *stagingBufferRegion = device.writeDataToStagingBuffer(writeInfo.data, writeInfo.size);
        const VkBufferCopy copyRegion = {
            .srcOffset = stagingBufferRegion->offset(),
            .dstOffset = offset() + writeInfo.offset,
            .size = writeInfo.size,
        };
        vkCmdCopyBuffer(commandBuffer, stagingBufferRegion->buffer(), buffer(), 1, &copyRegion);
    }

    if (!useSynchronizationType)
    {
        stageMask = writeInfo.sourceStageMask;
        accessMask = writeInfo.sourceAccessMask;
    }

    const LunaBufferMemoryBarrier postWriteBarrier = {
        .sourceStageMask = smallWrite ? VK_PIPELINE_STAGE_2_CLEAR_BIT : VK_PIPELINE_STAGE_2_COPY_BIT,
        .sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .destinationStageMask = stageMask,
        .destinationAccessMask = accessMask,
        .buffer = helpers::toHandle(this),
    };
    const LunaDependencyInfo postWriteDependencyInfo = {
        .bufferMemoryBarrierCount = 1,
        .bufferMemoryBarriers = &postWriteBarrier,
    };
    helpers::pipelineBarrier(commandBuffer, postWriteDependencyInfo);
    return VK_SUCCESS;
}
} // namespace luna

VkResult lunaCreateBuffer(const LunaDevice device, const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo != nullptr);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createBufferRegion(*creationInfo, buffer));
    return VK_SUCCESS;
}

void lunaDestroyBuffer(const LunaBuffer buffer)
{
    if (buffer == LUNA_NULL_HANDLE)
    {
        return;
    }
    luna::BufferRegion::destroy(*luna::helpers::fromHandle<luna::BufferRegion>(buffer));
}

VkResult lunaResizeBuffer(const LunaDevice device, LunaBuffer *buffer, const VkDeviceSize newSize)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(buffer && *buffer != LUNA_NULL_HANDLE);
    const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(*buffer);
    if (bufferRegion.size() == newSize)
    {
        return VK_SUCCESS;
    }
    const LunaBufferCreationInfo creationInfo = {
        .size = newSize,
        .alignment = bufferRegion.alignment(),
        .memoryType = bufferRegion.memoryType(),
        .additionalUsageFlags = bufferRegion.additionalUsageFlags(),
    };
    luna::BufferRegion::destroy(bufferRegion);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createBufferRegion(creationInfo, buffer));
    return VK_SUCCESS;
}

VkResult lunaFillBuffer(const LunaDevice device,
                        const LunaCommandBuffer commandBuffer,
                        const LunaBuffer buffer,
                        const uint32_t data)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(buffer != LUNA_NULL_HANDLE);

    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(static_cast<VkDevice>(deviceObject)));

    const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(buffer);
    vkCmdFillBuffer(commandBufferObject, bufferRegion.buffer(), bufferRegion.offset(), bufferRegion.size(), data);
    return VK_SUCCESS;
}

VkResult lunaWriteUintToBuffer(const LunaDevice device,
                               const LunaCommandBuffer commandBuffer,
                               const LunaBuffer buffer,
                               const VkDeviceSize offset,
                               const uint32_t value)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(buffer != LUNA_NULL_HANDLE);

    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(static_cast<VkDevice>(deviceObject)));

    const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(buffer);
    vkCmdFillBuffer(commandBufferObject,
                    bufferRegion.buffer(),
                    bufferRegion.offset() + offset,
                    sizeof(uint32_t),
                    value);
    return VK_SUCCESS;
}

VkResult lunaWriteDataToBuffer(const LunaDevice device,
                               const LunaCommandBuffer commandBuffer,
                               const LunaBuffer buffer,
                               const LunaBufferWriteInfo *writeInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(writeInfo != nullptr);
    if (writeInfo->size != 0)
    {
        assert(buffer != LUNA_NULL_HANDLE);
        assert(writeInfo->data != nullptr);

        luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(buffer);
        if (bufferRegion.data() == nullptr)
        {
            CHECK_RESULT_RETURN(bufferRegion.writeData(*luna::helpers::fromHandle<luna::Device>(device),
                                                       *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                                                       *writeInfo));
        } else
        {
            std::copy_n(static_cast<const char *>(writeInfo->data),
                        writeInfo->size,
                        bufferRegion.data() + writeInfo->offset);
        }
    }
    return VK_SUCCESS;
}

VkBuffer lunaGetVkBuffer(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::BufferRegion>(buffer)->buffer();
}

void *lunaGetBufferDataPointer(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::BufferRegion>(buffer)->data();
}

VkDeviceSize lunaGetBufferSize(const LunaBuffer buffer)
{
    if (buffer == LUNA_NULL_HANDLE)
    {
        return 0;
    }
    return luna::helpers::fromHandle<luna::BufferRegion>(buffer)->size();
}

VkDeviceSize lunaGetBufferOffset(const LunaBuffer buffer)
{
    assert(buffer != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::BufferRegion>(buffer)->offset();
}

VkDeviceAddress lunaGetBufferDeviceAddress(const LunaDevice device, const LunaBuffer buffer)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(buffer != LUNA_NULL_HANDLE);
    const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(buffer);
    if (bufferRegion.size() == 0)
    {
        return 0;
    }
    const VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = bufferRegion.buffer(),
    };
    return vkGetBufferDeviceAddress(lunaGetVkDevice(device), &bufferDeviceAddressInfo) + bufferRegion.offset();
}

void lunaClearStagingBuffer(const LunaDevice device)
{
    assert(device != LUNA_NULL_HANDLE);
    luna::helpers::fromHandle<luna::Device>(device)->clearStagingBuffer();
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
        const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(buffers[i]);
        buffersVector.emplace_back(bufferRegion.buffer());
        offsetsVector.emplace_back(bufferRegion.offset());
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

    const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(buffer);
    vkCmdBindIndexBuffer(commandBufferObject, bufferRegion.buffer(), bufferRegion.offset(), indexType);
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
    const luna::BufferRegion *bufferRegion = luna::helpers::fromHandle<luna::BufferRegion>(drawInfo->buffer);
    vkCmdDrawIndirect(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                      bufferRegion->buffer(),
                      bufferRegion->offset(),
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
    const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(drawInfo->buffer);
    vkCmdDrawIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                           bufferRegion.buffer(),
                           bufferRegion.offset() + sizeof(uint32_t),
                           bufferRegion.buffer(),
                           bufferRegion.offset(),
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
    const luna::BufferRegion *bufferRegion = luna::helpers::fromHandle<luna::BufferRegion>(drawInfo->buffer);
    vkCmdDrawIndexedIndirect(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                             bufferRegion->buffer(),
                             bufferRegion->offset(),
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
    const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(drawInfo->buffer);
    vkCmdDrawIndexedIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                                  bufferRegion.buffer(),
                                  bufferRegion.offset() + sizeof(uint32_t),
                                  bufferRegion.buffer(),
                                  bufferRegion.offset(),
                                  drawInfo->maxDrawCount,
                                  drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}
