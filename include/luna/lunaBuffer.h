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

VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer);
VkResult lunaCreateBuffers(uint32_t count, const LunaBufferCreationInfo *creationInfos, LunaBuffer **buffers);

void lunaDestroyBuffer(LunaBuffer buffer);

VkResult lunaResizeBuffer(LunaBuffer *buffer, VkDeviceSize newSize);

void lunaWriteDataToBuffer(LunaBuffer buffer, const void *data, size_t bytes, size_t offset);

LunaBufferCreationInfo lunaBufferGetCreationInfo(LunaBuffer buffer);

#ifdef __cplusplus
}
#endif

#endif //LUNA_LUNABUFFER_H
