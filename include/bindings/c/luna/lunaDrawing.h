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

VkResult lunaCreateRenderPass(LunaDevice device,
                              const LunaRenderPassCreationInfo *creationInfo,
                              LunaRenderPass *renderPass);
VkResult lunaCreateRenderPass2(LunaDevice device,
                               const LunaRenderPassCreationInfo2 *creationInfo,
                               LunaRenderPass *renderPass);
LunaRenderPassSubpass lunaGetRenderPassSubpassByName(LunaRenderPass renderPass, const char *name);

VkImage lunaGetRenderPassDepthImage(LunaRenderPass renderPass);

VkResult lunaBeginRenderPass(LunaDevice device,
                             LunaCommandBuffer commandBuffer,
                             LunaRenderPass renderPass,
                             const LunaRenderPassBeginInfo *beginInfo);
void lunaNextSubpass(LunaCommandBuffer commandBuffer);
void lunaEndRenderPass(LunaCommandBuffer commandBuffer);

VkResult lunaBindVertexBuffers(LunaDevice device,
                               LunaCommandBuffer commandBuffer,
                               const LunaBuffer *buffers,
                               uint32_t firstBinding,
                               uint32_t bindingCount);
VkResult lunaBindIndexBuffer(LunaDevice device,
                             LunaCommandBuffer commandBuffer,
                             LunaBuffer buffer,
                             VkIndexType indexType);

VkResult lunaDraw(LunaDevice device, LunaCommandBuffer commandBuffer, const LunaDrawInfo *drawInfo);
VkResult lunaDrawIndirect(LunaDevice device, LunaCommandBuffer commandBuffer, const LunaDrawIndirectInfo *drawInfo);
VkResult lunaDrawIndirectCount(LunaDevice device,
                               LunaCommandBuffer commandBuffer,
                               const LunaDrawIndirectCountInfo *drawInfo);

VkResult lunaDrawIndexed(LunaDevice device, LunaCommandBuffer commandBuffer, const LunaDrawIndexedInfo *drawInfo);
VkResult lunaDrawIndexedIndirect(LunaDevice device,
                                 LunaCommandBuffer commandBuffer,
                                 const LunaDrawIndexedIndirectInfo *drawInfo);
VkResult lunaDrawIndexedIndirectCount(LunaDevice device,
                                      LunaCommandBuffer commandBuffer,
                                      const LunaDrawIndexedIndirectCountInfo *drawInfo);

VkResult lunaDrawBuffer(LunaDevice device,
                        LunaCommandBuffer commandBuffer,
                        LunaBuffer vertexBuffer,
                        const LunaDrawInfo *drawInfo);
VkResult lunaDrawBufferIndirect(LunaDevice device,
                                LunaCommandBuffer commandBuffer,
                                LunaBuffer vertexBuffer,
                                const LunaDrawIndirectInfo *drawInfo);
VkResult lunaDrawBufferIndirectCount(LunaDevice device,
                                     LunaCommandBuffer commandBuffer,
                                     LunaBuffer vertexBuffer,
                                     const LunaDrawIndirectCountInfo *drawInfo);

VkResult lunaDrawBufferIndexed(LunaDevice device,
                               LunaCommandBuffer commandBuffer,
                               LunaBuffer vertexBuffer,
                               LunaBuffer indexBuffer,
                               VkIndexType indexType,
                               const LunaDrawIndexedInfo *drawInfo);
VkResult lunaDrawBufferIndexedIndirect(LunaDevice device,
                                       LunaCommandBuffer commandBuffer,
                                       LunaBuffer vertexBuffer,
                                       LunaBuffer indexBuffer,
                                       VkIndexType indexType,
                                       const LunaDrawIndexedIndirectInfo *drawInfo);
VkResult lunaDrawBufferIndexedIndirectCount(LunaDevice device,
                                            LunaCommandBuffer commandBuffer,
                                            LunaBuffer vertexBuffer,
                                            LunaBuffer indexBuffer,
                                            VkIndexType indexType,
                                            const LunaDrawIndexedIndirectCountInfo *drawInfo);

VkResult lunaBindDescriptorSets(LunaDevice device,
                                LunaCommandBuffer commandBuffer,
                                LunaGraphicsPipeline pipeline,
                                const LunaDescriptorSetBindInfo *bindInfo);

VkResult lunaPushConstants(LunaDevice device, LunaCommandBuffer commandBuffer, LunaGraphicsPipeline pipeline);

VkResult lunaResizeSwapchain(LunaDevice device, const LunaSwapchainResizeInfo *resizeInfo);

VkResult lunaBeginFrame(LunaDevice device, LunaCommandBuffer commandBuffer);
VkResult lunaEndFrame(LunaDevice device,
                      LunaCommandBuffer commandBuffer,
                      const LunaPresentInfo *presentInfo,
                      const LunaCommandBufferSubmitInfo *submitInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNADRAWING_H
