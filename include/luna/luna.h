//
// Created by NBT22 on 2/11/25.
//

#ifndef LUNA_H
#define LUNA_H

#ifdef __cplusplus
extern "C"
{
#endif

// NOLINTBEGIN(*-include-cleaner)
#include <luna/lunaBuffer.h>
#include <luna/lunaCommandBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaImage.h>
#include <luna/lunaInstance.h>
#include <luna/lunaTypes.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
// NOLINTEND(*-include-cleaner)

// TODO (0.3.0): Rename functions so that they are lunaObjectAction instead of lunaActionObject
// TODO (0.3.0): Check functions for missing parameter documentation

/**
 * @brief Create a new descriptor pool.
 * @param device
 * @param[in] creationInfo A pointer to the @c LunaDescriptorPoolCreationInfo structure containing information about how to create the descriptor pool.
 * @param[out] descriptorPool A pointer to the @c LunaDescriptorPool handle in which the resulting descriptor pool will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDescriptorPool.html
 */
VkResult lunaCreateDescriptorPool(LunaDevice device,
                                  const LunaDescriptorPoolCreationInfo *creationInfo,
                                  LunaDescriptorPool *descriptorPool);
/**
 * @brief Create a new descriptor set layout.
 * @param device
 * @param[in] creationInfo A pointer to the @c LunaDescriptorSetLayoutCreationInfo structure containing information about how to create the descriptor set layout.
 * @param[out] descriptorSetLayout A pointer to the @c LunaDescriptorSetLayout handle in which the resulting descriptor set layout will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDescriptorSetLayout.html
 */
VkResult lunaCreateDescriptorSetLayout(LunaDevice device,
                                       const LunaDescriptorSetLayoutCreationInfo *creationInfo,
                                       LunaDescriptorSetLayout *descriptorSetLayout);
/**
 * @brief Allocate a new descriptor set.
 * @param device
 * @param[in] allocationInfo A pointer to the @c LunaDescriptorSetAllocationInfo structure containing information about how the descriptor set should be allocated.
 * @param[out] descriptorSets A pointer to the @c LunaDescriptorSet handle in which the resulting descriptor set will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateDescriptorSets.html
 */
VkResult lunaAllocateDescriptorSets(LunaDevice device,
                                    const LunaDescriptorSetAllocationInfo *allocationInfo,
                                    LunaDescriptorSet *descriptorSets);
/**
 * @brief Update the contents of a descriptor set.
 * @param device
 * @param[in] descriptorWriteCount The number of elements in the @c descriptorWrites array.
 * @param[in] descriptorWrites An array of @c LunaWriteDescriptorSet structures containing information about what to write.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkUpdateDescriptorSets.html
 */
VkResult lunaWriteDescriptorSets(LunaDevice device,
                                 uint32_t descriptorWriteCount,
                                 const LunaWriteDescriptorSet *descriptorWrites);

/**
 * @brief Create a new shader module.
 * @param device
 * @param[in] creationInfo A pointer to the @c LunaShaderModuleCreationInfo structure containing information about how to create the shader module.
 * @param[out] shaderModule A pointer to the @c LunaShaderModule handle in which the resulting shader module will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateShaderModule.html
 */
VkResult lunaCreateShaderModule(LunaDevice device,
                                const LunaShaderModuleCreationInfo *creationInfo,
                                LunaShaderModule *shaderModule);

// TODO (0.3.0): Add lunaDestroyShaderModule

/**
 * @brief Create a new graphics pipeline.
 * @param device
 * @param[in] creationInfo A pointer to the @c LunaGraphicsPipelineCreationInfo structure containing information about how to create the graphics pipeline.
 * @param[out] pipeline A pointer to the @c LunaGraphicsPipeline handle in which the resulting graphics pipeline will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateGraphicsPipelines.html
 */
VkResult lunaCreateGraphicsPipeline(LunaDevice device,
                                    const LunaGraphicsPipelineCreationInfo *creationInfo,
                                    LunaGraphicsPipeline *pipeline);

/**
 * @brief Create a new graphics pipeline.
 * @param[in] creationInfo A pointer to the @c LunaGraphicsPipelineCreationInfo structure containing information about how to create the graphics pipeline.
 * @param[out] pipeline A pointer to the @c LunaGraphicsPipeline handle in which the resulting graphics pipeline will be returned.
 */
VkResult lunaCreateGraphicsPipelineUsingReflection(const LunaGraphicsPipelineUsingReflectionCreationInfo *creationInfo,
                                                   LunaGraphicsPipeline *pipeline);


/**
 * @brief Create a new compute pipeline.
 * @param device
 * @param[in] creationInfo A pointer to the @c LunaComputePipelineCreationInfo structure containing information about how to create the compute pipeline.
 * @param[out] pipeline A pointer to the @c LunaComputePipeline handle in which the resulting compute pipeline will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateComputePipeline.html
 */
VkResult lunaCreateComputePipeline(LunaDevice device,
                                   const LunaComputePipelineCreationInfo *creationInfo,
                                   LunaComputePipeline *pipeline);
/**
 * @brief Dispatch a compute pipeline.
 * @param[in] info A pointer to the @c LunaDispatchInfo structure containing information about what to dispatch.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdDispatch.html
 */
VkResult lunaDispatch(LunaDevice device, LunaCommandBuffer commandBuffer, const LunaDispatchInfo *info);
/**
 * @brief Dispatch a compute pipeline, with potentially non-zero starting workgroup ids.
 * @param device
 * @param commandBuffer
 * @param[in] info A pointer to the @c LunaDispatchBaseInfo structure containing information about what to dispatch.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdDispatchBase.html
 */
VkResult lunaDispatchBase(LunaDevice device, LunaCommandBuffer commandBuffer, const LunaDispatchBaseInfo *info);

VkResult lunaBindComputePipeline(VkCommandBuffer commandBuffer,
                                 LunaComputePipeline pipeline,
                                 const LunaDescriptorSetBindInfo *descriptorSetBindInfo);

/**
 * @brief Insert a pipeline memory dependency
 * @param device
 * @param commandBuffer
 * @param[in] dependencyInfo
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdPipelineBarrier.html
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdPipelineBarrier2.html
 */
VkResult lunaPipelineBarrier(LunaDevice device,
                             LunaCommandBuffer commandBuffer,
                             const LunaDependencyInfo *dependencyInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNA_H
