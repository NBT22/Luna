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
 * @brief Create a new buffer.
 * @param[in] creationInfo A pointer to the @c LunaBufferCreationInfo structure containing information about how to create the buffer.
 * @param[out] buffer A pointer to the @c LunaBuffer handle in which the resulting buffer will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
 */
VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer);
/**
 * @brief Create an array of new buffers.
 * @param[in] count The number of elements in the @c creationInfos array.
 * @param[in] creationInfos An array of @c LunaBufferCreationInfo structures containing information about how to create the buffers.
 * @param[out] buffers An array of pointers to the @c LunaBuffer handles in which the resulting buffers will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
 */
VkResult lunaCreateBuffers(uint32_t count, const LunaBufferCreationInfo *creationInfos, LunaBuffer **buffers);

/**
 * @brief Destroy a buffer.
 * @param[in] buffer The @c LunaBuffer handle to destroy.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyBuffer.html
 */
void lunaDestroyBuffer(LunaBuffer buffer);

/**
 * @brief Ensure a buffer is at least @c size bytes, resizing if it is not.
 * @param[in,out] buffer A pointer to the @c LunaBuffer handle containing the buffer to resize.
 * @param[in] size The new size to make the buffer.
 */
VkResult lunaGrowBuffer(LunaBuffer *buffer, VkDeviceSize size);

/**
 * @brief Resize a buffer, keeping the contents intact.
 * @param[in,out] buffer A pointer to the @c LunaBuffer handle containing the buffer to resize.
 * @param[in] newSize The new size to make the buffer.
 */
VkResult lunaResizeBuffer(LunaBuffer *buffer, VkDeviceSize newSize);

VkResult lunaWriteDataToBuffer(LunaBuffer buffer, const LunaBufferWriteInfo *writeInfo);

VkDeviceSize lunaGetBufferSize(LunaBuffer buffer);
VkBufferCreateFlags lunaGetBufferCreationFlags(LunaBuffer buffer);
VkBufferUsageFlags lunaGetBufferUsageFlags(LunaBuffer buffer);
void lunaGetBufferAllocationCreateInfo(LunaBuffer buffer, VmaAllocationCreateInfo *allocationCreateInfo);
void lunaGetBufferCreationInfo(LunaBuffer buffer,
                               LunaBufferCreationInfo *creationInfo,
                               VmaAllocationCreateInfo *allocationCreateInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNA_LUNABUFFER_H
