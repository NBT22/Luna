//
// Created by NBT22 on 2/11/25.
//

#ifndef LUNA_H
#define LUNA_H

#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaImage.h>
#include <luna/lunaInstance.h>
#include <luna/lunaPipeline.h>
#include <luna/lunaRenderPass.h>
#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

void lunaDestroyInstance();

void lunaCreateDescriptorPool(const LunaDescriptorPoolCreationInfo *creationInfo, LunaDescriptorPool *descriptorPool);
void lunaCreateDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo *creationInfo,
								   LunaDescriptorSetLayout *descriptorSetLayout);
void lunaAllocateDescriptorSets(const LunaDescriptorSetAllocationInfo *allocationInfo,
								LunaDescriptorSet *descriptorSets);
void lunaWriteDescriptorSets(uint32_t writeCount, const LunaWriteDescriptorSet *descriptorWrites);

void lunaCreateShaderModule(const uint32_t *spirv, size_t bytes, VkShaderModule *shaderModule);

void lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo);
void lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer);
void lunaWriteDataToBuffer(LunaBuffer buffer, const void *data, size_t bytes);

#ifdef __cplusplus
}
#endif

#endif //LUNA_H
