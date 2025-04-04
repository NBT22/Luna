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

VkResult lunaCreateDescriptorPool(const LunaDescriptorPoolCreationInfo *creationInfo,
								  LunaDescriptorPool *descriptorPool);
VkResult lunaCreateDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo *creationInfo,
									   LunaDescriptorSetLayout *descriptorSetLayout);
VkResult lunaAllocateDescriptorSets(const LunaDescriptorSetAllocationInfo *allocationInfo,
									LunaDescriptorSet *descriptorSets);
void lunaWriteDescriptorSets(uint32_t writeCount, const LunaWriteDescriptorSet *descriptorWrites);

VkResult lunaCreateShaderModule(const uint32_t *spirv, size_t bytes, VkShaderModule *shaderModule);

VkResult lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo);
VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer);
void lunaWriteDataToBuffer(LunaBuffer buffer, const void *data, size_t bytes);

#ifdef __cplusplus
}
#endif

#endif //LUNA_H
