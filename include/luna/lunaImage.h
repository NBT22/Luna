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

LunaSampler lunaCreateSampler(const LunaSamplerCreationInfo *creationInfo);

LunaImage lunaCreateImage(const LunaSampledImageCreationInfo *creationInfo);
LunaImage lunaCreateImageArray(const LunaSampledImageCreationInfo *creationInfo, uint32_t arrayLayers);
LunaImage lunaCreateImage3D(const LunaSampledImageCreationInfo *creationInfo, uint32_t depth);
LunaImage lunaCreateImage3DArray(const LunaSampledImageCreationInfo *creationInfo,
								 uint32_t depth,
								 uint32_t arrayLayers);

#ifdef __cplusplus
}
#endif

#endif //LUNAIMAGE_H
