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
 * @param[in] creationInfo A pointer to the @c LunaBufferCreationInfo structure containing information about how to create the buffer.
 * @param[out] buffer A pointer to the @c LunaBuffer handle in which the resulting buffer will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
 */
VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer);

/**
 * @brief Destroy a buffer.
 * @param[in] buffer The @c LunaBuffer handle to destroy.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyBuffer.html
 */
void lunaDestroyBuffer(LunaBuffer buffer);

/**
 * @brief Get the list of regions
 * @param[in] buffer The @c LunaBuffer to get the regions of.
 * @param[out] count The size of the @c LunaBufferRegion list returned by the function.
 * @return A list of @code count@endcode @c LunaBufferRegion handles.
 */
LunaBufferRegion *lunaGetBufferRegions(LunaBuffer buffer, uint32_t *count);

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

VkResult lunaFillBuffer(LunaBuffer buffer, uint32_t data);

VkResult lunaCreateBufferView(const LunaBufferViewCreationInfo *creationInfo, LunaBufferView *bufferView);

VkResult lunaWriteDataToBuffer(LunaBuffer buffer, const LunaBufferWriteInfo *writeInfo);
void *lunaGetBufferDataPointer(LunaBuffer buffer);

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
