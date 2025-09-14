//
// Created by NBT22 on 2/11/25.
//

#ifndef LUNA_H
#define LUNA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaDevice.h> // NOLINT(*-include-cleaner)
#include <luna/lunaDrawing.h> // NOLINT(*-include-cleaner)
#include <luna/lunaImage.h> // NOLINT(*-include-cleaner)
#include <luna/lunaInstance.h> // NOLINT(*-include-cleaner)
#include <luna/lunaPipeline.h> // NOLINT(*-include-cleaner)
#include <luna/lunaRenderPass.h> // NOLINT(*-include-cleaner)
#include <luna/lunaTypes.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

VkResult lunaCreateDescriptorPool(const LunaDescriptorPoolCreationInfo *creationInfo,
                                  LunaDescriptorPool *descriptorPool);
VkResult lunaCreateDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo *creationInfo,
                                       LunaDescriptorSetLayout *descriptorSetLayout);
VkResult lunaAllocateDescriptorSets(const LunaDescriptorSetAllocationInfo *allocationInfo,
                                    LunaDescriptorSet *descriptorSets);
void lunaWriteDescriptorSets(uint32_t writeCount, const LunaWriteDescriptorSet *descriptorWrites);
void lunaDestroyDescriptorSet(LunaDescriptorSet descriptorSet);

VkResult lunaCreateShaderModule(const LunaShaderModuleCreationInfo *creationInfo, LunaShaderModule *shaderModule);

VkResult lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo);
VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer);
VkResult lunaCreateBuffers(uint32_t count, const LunaBufferCreationInfo *creationInfos, LunaBuffer **buffers);
void lunaDestroyBuffer(LunaBuffer buffer);
VkResult lunaResizeBuffer(LunaBuffer buffer, VkDeviceSize newSize);
void lunaWriteDataToBuffer(LunaBuffer buffer, const void *data, size_t bytes, size_t offset);

VkResult lunaCreateCommandPool(const LunaCommandPoolCreationInfo *creationInfo, LunaCommandPool *commandPool);
VkResult lunaResetCommandPool(LunaCommandPool commandPool, VkCommandPoolResetFlagBits flags);

#ifdef __cplusplus
}
#endif

#endif //LUNA_H
