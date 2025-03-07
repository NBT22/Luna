//
// Created by NBT22 on 2/11/25.
//

#ifndef LUNA_H
#define LUNA_H

#include <luna/lunaDevice.h>
#include <luna/lunaInstance.h>
#include <luna/lunaPipeline.h>
#include <luna/lunaRenderPass.h>
#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

VkShaderModule lunaCreateShaderModule(const uint32_t *spirv, size_t bytes);

void lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo);
LunaBuffer lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNA_H
