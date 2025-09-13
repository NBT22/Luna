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

VkResult lunaCreateImage(const LunaSampledImageCreationInfo *creationInfo, LunaImage *image);
VkResult lunaCreateImageArray(const LunaSampledImageCreationInfo *creationInfo, uint32_t arrayLayers, LunaImage *image);
VkResult lunaCreateImage3D(const LunaSampledImageCreationInfo *creationInfo, uint32_t depth, LunaImage *image);
VkResult lunaCreateImage3DArray(const LunaSampledImageCreationInfo *creationInfo,
                                uint32_t depth,
                                uint32_t arrayLayers,
                                LunaImage *image);

void lunaDestroyImage(LunaImage image);

#ifdef __cplusplus
}
#endif

#endif //LUNAIMAGE_H
