//
// Created by NBT22 on 3/12/25.
//

#ifndef LUNAIMAGE_H
#define LUNAIMAGE_H

#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

void lunaCreateSampler(const LunaSamplerCreationInfo *creationInfo, LunaSampler *sampler);

void lunaCreateImage(const LunaSampledImageCreationInfo *creationInfo, LunaImage *image);
void lunaCreateImageArray(const LunaSampledImageCreationInfo *creationInfo, uint32_t arrayLayers, LunaImage *image);
void lunaCreateImage3D(const LunaSampledImageCreationInfo *creationInfo, uint32_t depth, LunaImage *image);
void lunaCreateImage3DArray(const LunaSampledImageCreationInfo *creationInfo,
							uint32_t depth,
							uint32_t arrayLayers,
							LunaImage *image);

#ifdef __cplusplus
}
#endif

#endif //LUNAIMAGE_H
