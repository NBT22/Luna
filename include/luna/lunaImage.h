//
// Created by NBT22 on 3/12/25.
//

#ifndef LUNAIMAGE_H
#define LUNAIMAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

VkResult lunaCreateSampler(const LunaSamplerCreationInfo *creationInfo, LunaSampler *sampler);
void lunaDestroySampler(LunaSampler sampler);

VkResult lunaCreateImage(const LunaImageCreationInfo *creationInfo, LunaImage *image);
VkResult lunaCreateImageArray(const LunaImageCreationInfo *creationInfo, uint32_t arrayLayers, LunaImage *image);
VkResult lunaCreateImage3D(const LunaImageCreationInfo *creationInfo, uint32_t depth, LunaImage *image);
VkResult lunaCreateImage3DArray(const LunaImageCreationInfo *creationInfo,
                                uint32_t depth,
                                uint32_t arrayLayers,
                                LunaImage *image);

VkResult lunaUpdateImage(LunaImage image, const LunaImageWriteInfo *writeInfo);

// TODO (0.3.0): Finalize this function (maybe allow for taking multiple images and regions)
VkResult lunaBlitImageToSwapchain(LunaImage image, const VkImageBlit2 *blitRegion);

VkResult lunaCopyImageToBuffer(LunaImage image,
                               LunaBuffer buffer,
                               uint32_t regionCount,
                               const VkBufferImageCopy *regions);

void lunaDestroyImage(LunaImage image);

#ifdef __cplusplus
}
#endif

#endif //LUNAIMAGE_H
