//
// Created by NBT22 on 2/11/25.
//

#ifndef LUNA_H
#define LUNA_H

#include <luna/lunaDevice.h>
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

LunaDescriptorPool lunaCreateDescriptorPool(const LunaDescriptorPoolCreationInfo *creationInfo);
LunaDescriptorSetLayout lunaCreateDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo *creationInfo);
void lunaAllocateDescriptorSets(const LunaDescriptorSetAllocationInfo *allocationInfo,
								LunaDescriptorSet *descriptorSets);
void lunaWriteDescriptorSets(uint32_t writeCount, const LunaWriteDescriptorSet *descriptorWrites);

VkShaderModule lunaCreateShaderModule(const uint32_t *spirv, size_t bytes);

void lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo);
LunaBuffer lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo);
void lunaWriteDataToBuffer(LunaBuffer buffer, const void *data, size_t bytes);

void lunaDrawBuffer(const LunaVertexBufferDrawInfo *drawInfo);
void lunaDrawFrame();

#ifdef __cplusplus
}
#endif

#endif //LUNA_H
