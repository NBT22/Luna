//
// Created by NBT22 on 9/26/25.
//

#ifndef LUNA_LUNABUFFER_H
#define LUNA_LUNABUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

/**
 * @brief Create a new buffer, optionally with dedicated regions.
 * @param device
 * @param[in] creationInfo A pointer to the @c LunaBufferCreationInfo structure containing information about how to create the buffer.
 * @param[out] buffer A pointer to the @c LunaBuffer handle in which the resulting buffer will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
 */
VkResult lunaCreateBuffer(LunaDevice device, const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer);

/**
 * @brief Destroy a buffer.
 * @param device
 * @param[in] buffer The @c LunaBuffer handle to destroy.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyBuffer.html
 */
void lunaDestroyBuffer(LunaDevice device, LunaBuffer buffer);

/**
 * @brief Ensure a buffer is at least @c size bytes, resizing if it is not.
 * @param[in,out] buffer A pointer to the @c LunaBuffer handle containing the buffer to resize.
 * @param[in] size The new size to make the buffer.
 */
VkResult lunaGrowBuffer(LunaDevice device, LunaBuffer *buffer, VkDeviceSize size);

/**
 * @brief Resize a buffer, keeping the contents intact.
 * @param[in,out] buffer A pointer to the @c LunaBuffer handle containing the buffer to resize.
 * @param[in] newSize The new size to make the buffer.
 */
VkResult lunaResizeBuffer(LunaDevice device, LunaBuffer *buffer, VkDeviceSize newSize);

VkResult lunaFillBuffer(LunaDevice device,
                        LunaCommandBuffer commandBuffer,
                        LunaBuffer buffer,
                        uint32_t data,
                        const LunaCommandBufferSubmitInfo *submitInfo);

VkResult lunaCreateBufferView(LunaDevice device,
                              const LunaBufferViewCreationInfo *creationInfo,
                              LunaBufferView *bufferView);

// TODO (0.3.0): This currently REQUIRES a queue submission for every call if writing to non-mapped VRAM
//  This behavior is caused by always using the same buffer for transfer, and therefore overwriting the contents
VkResult lunaWriteDataToBuffer(LunaDevice device,
                               LunaCommandBuffer commandBuffer,
                               LunaBuffer buffer,
                               const LunaBufferWriteInfo *writeInfo);
void *lunaGetBufferDataPointer(LunaBuffer buffer);

VkDeviceSize lunaGetBufferSize(LunaBuffer buffer);
VkBufferCreateFlags lunaGetBufferCreationFlags(LunaBuffer buffer);
VkBufferUsageFlags lunaGetBufferUsageFlags(LunaBuffer buffer);
void lunaGetBufferAllocationCreateInfo(LunaBuffer buffer, VmaAllocationCreateInfo *allocationCreateInfo);
void lunaGetBufferCreationInfo(LunaBuffer buffer,
                               LunaBufferCreationInfo *creationInfo,
                               VmaAllocationCreateInfo *allocationCreateInfo);
VkDeviceAddress lunaGetBufferDeviceAddress(LunaDevice device, LunaBuffer buffer);
VkBuffer lunaGetVkBuffer(LunaBuffer buffer);
VkDeviceSize lunaGetBufferOffset(LunaBuffer buffer);

#ifdef __cplusplus
}
#endif

#endif //LUNA_LUNABUFFER_H
