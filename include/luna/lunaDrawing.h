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

void lunaBindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           const LunaBuffer *buffers,
                           const VkDeviceSize *offsets);

VkResult lunaDrawBuffer(LunaBuffer vertexBuffer,
                        LunaGraphicsPipeline pipeline,
                        const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                        uint32_t vertexCount,
                        uint32_t instanceCount,
                        uint32_t firstVertex,
                        uint32_t firstInstance);
VkResult lunaDrawBufferIndirect(LunaBuffer vertexBuffer,
                                LunaGraphicsPipeline pipeline,
                                const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                LunaBuffer buffer,
                                VkDeviceSize offset,
                                uint32_t drawCount,
                                uint32_t stride);
VkResult lunaDrawBufferIndirectCount(LunaBuffer vertexBuffer,
                                     LunaGraphicsPipeline pipeline,
                                     const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                     LunaBuffer buffer,
                                     VkDeviceSize offset,
                                     LunaBuffer countBuffer,
                                     VkDeviceSize countBufferOffset,
                                     uint32_t maxDrawCount,
                                     uint32_t stride);

VkResult lunaDrawBufferIndexed(LunaBuffer vertexBuffer,
                               LunaBuffer indexBuffer,
                               VkIndexType indexType,
                               LunaGraphicsPipeline pipeline,
                               const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                               uint32_t indexCount,
                               uint32_t instanceCount,
                               uint32_t firstIndex,
                               int32_t vertexOffset,
                               uint32_t firstInstance);
VkResult lunaDrawBufferIndexedIndirect(LunaBuffer vertexBuffer,
                                       LunaBuffer indexBuffer,
                                       VkIndexType indexType,
                                       LunaGraphicsPipeline pipeline,
                                       const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                       LunaBuffer buffer,
                                       VkDeviceSize offset,
                                       uint32_t drawCount,
                                       uint32_t stride);
VkResult lunaDrawBufferIndexedIndirectCount(LunaBuffer vertexBuffer,
                                            LunaBuffer indexBuffer,
                                            VkIndexType indexType,
                                            LunaGraphicsPipeline pipeline,
                                            const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
                                            LunaBuffer buffer,
                                            VkDeviceSize offset,
                                            LunaBuffer countBuffer,
                                            VkDeviceSize countBufferOffset,
                                            uint32_t maxDrawCount,
                                            uint32_t stride);

// TODO: This should take a pipeline layout, but those don't exist yet
void lunaBindDescriptorSets(LunaGraphicsPipeline pipeline, const LunaDescriptorSetBindInfo *bindInfo);
VkResult lunaPushConstants(LunaGraphicsPipeline pipeline);
VkResult lunaResizeSwapchain(uint32_t renderPassResizeInfoCount,
                             const LunaRenderPassResizeInfo *renderPassResizeInfos,
                             const VkExtent2D *targetExtent,
                             VkExtent2D *newSwapchainExtent);
VkResult lunaPresentSwapchain(void);

#ifdef __cplusplus
}
#endif

#endif //LUNADRAWING_H
