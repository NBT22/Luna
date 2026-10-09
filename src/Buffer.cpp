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
Buffer::Buffer(const LunaMemoryType memoryType, const VmaAllocator &allocator):
    allocator_(allocator),
    memoryType_(memoryType)
{
    VmaAllocationInfo allocationInfo;
    CHECK_RESULT_THROW(vmaCreateBuffer(allocator,
                                       &BUFFER_CREATE_INFO,
                                       &allocationCreateInfo(memoryType),
                                       &buffer_,
                                       &allocation_,
                                       &allocationInfo));
    static constexpr VmaVirtualBlockCreateInfo VIRTUAL_BLOCK_CREATE_INFO = {
        .size = BUFFER_CREATE_INFO.size,
    };
    CHECK_RESULT_THROW(vmaCreateVirtualBlock(&VIRTUAL_BLOCK_CREATE_INFO, &virtualBlock_));
    data_ = allocationInfo.pMappedData;
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
    const LunaBufferCreationInfo creationInfo = {
        .size = newSize,
        .alignment = bufferRegion.alignment(),
        .memoryType = bufferRegion.memoryType(),
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
    if (writeInfo->bytes != 0)
    {
        assert(buffer != LUNA_NULL_HANDLE);
        assert(writeInfo->data != nullptr);

        const luna::BufferRegion &bufferRegion = *luna::helpers::fromHandle<luna::BufferRegion>(buffer);
        assert(bufferRegion.data() != nullptr); // TODO: Add support for non-mapped memory
        std::copy_n(static_cast<const char *>(writeInfo->data),
                    writeInfo->bytes,
                    bufferRegion.data() + writeInfo->offset);
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

void clearStagingBuffer(const LunaDevice device)
{
    assert(device != LUNA_NULL_HANDLE);
    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    deviceObject.stagingBuffer().clearRegions();
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
