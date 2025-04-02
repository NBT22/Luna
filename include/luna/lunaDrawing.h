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

void lunaDrawBuffer(LunaBuffer vertexBuffer,
					LunaGraphicsPipeline pipeline,
					const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
					uint32_t vertexCount,
					uint32_t instanceCount,
					uint32_t firstVertex,
					uint32_t firstInstance);
void lunaDrawBufferIndirect(LunaBuffer vertexBuffer,
							LunaGraphicsPipeline pipeline,
							const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
							LunaBuffer buffer,
							VkDeviceSize offset,
							uint32_t drawCount,
							uint32_t stride);
void lunaDrawBufferIndirectCount(LunaBuffer vertexBuffer,
								 LunaGraphicsPipeline pipeline,
								 const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
								 LunaBuffer buffer,
								 VkDeviceSize offset,
								 LunaBuffer countBuffer,
								 VkDeviceSize countBufferOffset,
								 uint32_t maxDrawCount,
								 uint32_t stride);

void lunaDrawBufferIndexed(LunaBuffer vertexBuffer,
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
void lunaDrawBufferIndexedIndirect(LunaBuffer vertexBuffer,
								   LunaBuffer indexBuffer,
								   VkDeviceSize indexOffset,
								   VkIndexType indexType,
								   LunaGraphicsPipeline pipeline,
								   const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
								   LunaBuffer buffer,
								   VkDeviceSize offset,
								   uint32_t drawCount,
								   uint32_t stride);
void lunaDrawBufferIndexedIndirectCount(LunaBuffer vertexBuffer,
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

void lunaPushConstants(LunaGraphicsPipeline pipeline);
void lunaDrawFrame();

#ifdef __cplusplus
}
#endif

#endif //LUNADRAWING_H
