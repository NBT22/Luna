//
// Created by NBT22 on 4/1/25.
//

#ifndef LUNADRAWING_H
#define LUNADRAWING_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

VkResult lunaCreateRenderPass(const LunaRenderPassCreationInfo *creationInfo, LunaRenderPass *renderPass);
VkResult lunaCreateRenderPass2(const LunaRenderPassCreationInfo2 *creationInfo, LunaRenderPass *renderPass);
LunaRenderPassSubpass lunaGetRenderPassSubpassByName(LunaRenderPass renderPass, const char *name);

VkResult lunaBeginRenderPass(LunaRenderPass renderPass, const LunaRenderPassBeginInfo *beginInfo);
void lunaNextSubpass(void);
void lunaEndRenderPass(void);

void lunaBindVertexBuffers(const LunaBuffer *buffers, uint32_t firstBinding, uint32_t bindingCount);
void lunaBindIndexBuffer(LunaBuffer buffer, VkIndexType indexType);

VkResult lunaDraw(const LunaDrawInfo *drawInfo);
VkResult lunaDrawIndirect(const LunaDrawIndirectInfo *drawInfo);
VkResult lunaDrawIndirectCount(const LunaDrawIndirectCountInfo *drawInfo);

VkResult lunaDrawIndexed(const LunaDrawIndexedInfo *drawInfo);
VkResult lunaDrawIndexedIndirect(const LunaDrawIndexedIndirectInfo *drawInfo);
VkResult lunaDrawIndexedIndirectCount(const LunaDrawIndexedIndirectCountInfo *drawInfo);

VkResult lunaDrawBuffer(LunaBuffer vertexBuffer, const LunaDrawInfo *drawInfo);
VkResult lunaDrawBufferIndirect(LunaBuffer vertexBuffer, const LunaDrawIndirectInfo *drawInfo);
VkResult lunaDrawBufferIndirectCount(LunaBuffer vertexBuffer, const LunaDrawIndirectCountInfo *drawInfo);

VkResult lunaDrawBufferIndexed(LunaBuffer vertexBuffer,
                               LunaBuffer indexBuffer,
                               VkIndexType indexType,
                               const LunaDrawIndexedInfo *drawInfo);
VkResult lunaDrawBufferIndexedIndirect(LunaBuffer vertexBuffer,
                                       LunaBuffer indexBuffer,
                                       VkIndexType indexType,
                                       const LunaDrawIndexedIndirectInfo *drawInfo);
VkResult lunaDrawBufferIndexedIndirectCount(LunaBuffer vertexBuffer,
                                            LunaBuffer indexBuffer,
                                            VkIndexType indexType,
                                            const LunaDrawIndexedIndirectCountInfo *drawInfo);

// TODO: This should take a pipeline layout, but those don't exist yet
void lunaBindDescriptorSets(LunaGraphicsPipeline pipeline, const LunaDescriptorSetBindInfo *bindInfo);

VkResult lunaPushConstants(LunaGraphicsPipeline pipeline);

VkResult lunaResizeSwapchain(uint32_t renderPassResizeInfoCount,
                             const LunaRenderPassResizeInfo *renderPassResizeInfos,
                             const VkExtent2D *targetExtent,
                             VkExtent2D *newSwapchainExtent);

VkResult lunaBeginFrame(bool allowSuboptimalSwapchain);
VkResult lunaEndFrame(void);

#ifdef __cplusplus
}
#endif

#endif //LUNADRAWING_H
