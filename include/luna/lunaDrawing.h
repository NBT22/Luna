//
// Created by NBT22 on 4/1/25.
//

#ifndef LUNADRAWING_H
#define LUNADRAWING_H

#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

// TODO: Vertex buffer offset
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
							   VkDeviceSize indexOffset,
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
									   VkDeviceSize indexOffset,
									   VkIndexType indexType,
									   LunaGraphicsPipeline pipeline,
									   const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
									   LunaBuffer buffer,
									   VkDeviceSize offset,
									   uint32_t drawCount,
									   uint32_t stride);
VkResult lunaDrawBufferIndexedIndirectCount(LunaBuffer vertexBuffer,
											LunaBuffer indexBuffer,
											VkDeviceSize indexOffset,
											VkIndexType indexType,
											LunaGraphicsPipeline pipeline,
											const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
											LunaBuffer buffer,
											VkDeviceSize offset,
											LunaBuffer countBuffer,
											VkDeviceSize countBufferOffset,
											uint32_t maxDrawCount,
											uint32_t stride);

VkResult lunaPushConstants(LunaGraphicsPipeline pipeline);
VkResult lunaPresentSwapChain();

#ifdef __cplusplus
}
#endif

#endif //LUNADRAWING_H
