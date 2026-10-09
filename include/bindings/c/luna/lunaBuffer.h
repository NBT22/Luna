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
 * @param[in] buffer The @c LunaBuffer handle to destroy.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyBuffer.html
 */
void lunaDestroyBuffer(LunaBuffer buffer);

/**
 * @brief Resize a buffer.
 * @warning The buffer's contents will be undefined after calling this function
 * @param[in,out] buffer A pointer to the @c LunaBuffer handle containing the buffer to resize.
 * @param[in] newSize The new size to make the buffer.
 */
VkResult lunaResizeBuffer(LunaDevice device, LunaBuffer *buffer, VkDeviceSize newSize);

VkResult lunaFillBuffer(LunaDevice device, LunaCommandBuffer commandBuffer, LunaBuffer buffer, uint32_t data);

VkResult lunaWriteUintToBuffer(LunaDevice device,
                               LunaCommandBuffer commandBuffer,
                               LunaBuffer buffer,
                               VkDeviceSize offset,
                               uint32_t value);

VkResult lunaWriteDataToBuffer(LunaDevice device,
                               LunaCommandBuffer commandBuffer,
                               LunaBuffer buffer,
                               const LunaBufferWriteInfo *writeInfo);

VkBuffer lunaGetVkBuffer(LunaBuffer buffer);
void *lunaGetBufferDataPointer(LunaBuffer buffer);
VkDeviceSize lunaGetBufferSize(LunaBuffer buffer);
VkDeviceSize lunaGetBufferOffset(LunaBuffer buffer);
VkDeviceAddress lunaGetBufferDeviceAddress(LunaDevice device, LunaBuffer buffer);

// TODO: Better solution for this
void clearStagingBuffer(LunaDevice device);

#ifdef __cplusplus
}
#endif

#endif //LUNA_LUNABUFFER_H
