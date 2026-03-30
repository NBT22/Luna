//
// Created by NBT22 on 3/28/26.
//

#ifndef LUNA_LUNACOMMANDBUFFER_H
#define LUNA_LUNACOMMANDBUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <stddef.h>
#include <vulkan/vulkan_core.h>

/**
 * @brief Create a new command pool.
 * @param[in] creationInfo A pointer to the @c LunaCommandPoolCreationInfo structure containing information about how to create the command pool.
 * @param[out] commandPool A pointer to the @c LunaCommandPool handle in which the resulting command pool will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateCommandPool.html
 */
VkResult lunaCreateCommandPool(const LunaCommandPoolCreationInfo *creationInfo, LunaCommandPool *commandPool);
/**
 * @brief Reset a command pool.
 * @param[in] commandPool The command pool to reset.
 * @param[in] flags A bitmask of @c VkCommandPoolResetFlagBits specifying options for how to reset the command pool.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkResetCommandPool.html
 * @warning This function waits for fences with a timeout of @c UINT64_MAX.
 */
VkResult lunaResetCommandPool(LunaCommandPool commandPool, VkCommandPoolResetFlags flags);
/**
 * @brief Reset a command pool.
 * @param[in] commandPool The command pool to reset.
 * @param[in] flags A bitmask of @c VkCommandPoolResetFlagBits specifying options for how to reset the command pool.
 * @param[in] timeout The time, in nanoseconds, to block for when waiting for fences.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkResetCommandPool.html
 */
VkResult lunaResetCommandPoolWithTimeout(LunaCommandPool commandPool, VkCommandPoolResetFlags flags, size_t timeout);

VkResult lunaAllocateCommandBuffer(const LunaCommandBufferAllocationInfo *allocationInfo,
                                   LunaCommandBuffer *commandBuffer);

VkResult lunaBeginCommandBuffer(LunaCommandBuffer commandBuffer);

VkResult lunaEndCommandBuffer(LunaCommandBuffer commandBuffer);

VkResult lunaResetCommandBuffer(LunaCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);

void lunaDestroyCommandBuffer(LunaCommandBuffer commandBuffer);

#ifdef __cplusplus
}
#endif

#endif //LUNA_LUNACOMMANDBUFFER_H
